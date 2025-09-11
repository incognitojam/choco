#!/usr/bin/env python3

import argparse
import json
import subprocess
import sys
from pathlib import Path

# Configuration
TARGET = "build/main"
EXAMPLES_DIR = "examples"
SNAPSHOTS_DIR = "snapshots"

# Colors
GREEN = '\033[0;32m'
RED = '\033[0;31m'
YELLOW = '\033[1;33m'
BLUE = '\033[0;34m'
CYAN = '\033[0;36m'
NC = '\033[0m'

def run_program(fixture_path):
  """Run the program and return exit code, stdout, stderr"""
  try:
    result = subprocess.run([TARGET, fixture_path],
                            capture_output=True, text=True, timeout=10)
    return {
      'exit_code': result.returncode,
      'stdout': result.stdout,
      'stderr': result.stderr,
      'status': 'pass' if result.returncode == 0 else 'fail'
    }
  except subprocess.TimeoutExpired:
    return {
      'exit_code': 124,  # timeout exit code
      'stdout': '',
      'stderr': 'Test timed out after 10 seconds',
      'status': 'fail'
    }
  except Exception as e:
    return {
      'exit_code': 1,
      'stdout': '',
      'stderr': f'Error running test: {e}',
      'status': 'fail'
    }

def load_snapshot(snapshot_file):
  """Load snapshot from file, handling both old and new formats"""
  if not snapshot_file.exists():
    return None

  content = snapshot_file.read_text().strip()

  # Try to parse as JSON (new format)
  try:
    return json.loads(content)
  except json.JSONDecodeError:
    # Old format - just stdout
    return {
      'exit_code': 0,
      'stdout': content,
      'stderr': '',
      'status': 'pass' if content else 'fail'
    }

def save_snapshot(snapshot_file, result):
  """Save result as JSON snapshot"""
  with open(snapshot_file, 'w') as f:
    json.dump(result, f, indent=2)

def compare_results(expected, actual):
  """Compare two results and return differences"""
  diffs = []

  if expected['exit_code'] != actual['exit_code']:
    diffs.append(f"Exit code: expected {expected['exit_code']}, got {actual['exit_code']}")

  if expected['stdout'] != actual['stdout']:
    diffs.append("Stdout differs")

  if expected['stderr'] != actual['stderr']:
    diffs.append("Stderr differs")

  return diffs

def run_test(fixture_path, verbose=False):
  """Run a single test, return (snapshot_matches, status_changed, result)"""
  basename = Path(fixture_path).stem
  snapshot_file = Path(SNAPSHOTS_DIR) / f"{basename}.js.snap"

  # Run the program
  actual = run_program(fixture_path)

  # Generate snapshot if missing
  if not snapshot_file.exists():
    print(f"{YELLOW}Generating missing snapshot for {basename}.js...{NC}")
    save_snapshot(snapshot_file, actual)
    status_icon = "✓" if actual['status'] == 'pass' else "✗"
    color = GREEN if actual['status'] == 'pass' else RED
    print(f"{color}{status_icon} Generated snapshot for {basename}.js ({actual['status']}){NC}")
    return True, False, actual  # New snapshot always "matches"

  # Load expected result
  expected = load_snapshot(snapshot_file)

  # Compare results
  diffs = compare_results(expected, actual)
  snapshot_matches = len(diffs) == 0
  status_changed = expected['status'] != actual['status']

  # Print result
  if snapshot_matches:
    print(f"{GREEN}✓ Test passed: {basename}.js{NC}")
  else:
    change_indicator = ""
    if status_changed:
      if actual['status'] == 'pass':
        change_indicator = f" {CYAN}(now passing!){NC}"
      else:
        change_indicator = f" {YELLOW}(now failing!){NC}"

    print(f"{RED}✗ Snapshot differs: {basename}.js{change_indicator}{NC}")

    if verbose:
      for diff in diffs:
        print(f"  {diff}")

      if 'Stdout differs' in diffs:
        print("Expected stdout:")
        print(expected['stdout'])
        print("Actual stdout:")
        print(actual['stdout'])

      if 'Stderr differs' in diffs:
        print("Expected stderr:")
        print(expected['stderr'])
        print("Actual stderr:")
        print(actual['stderr'])

      print("---")

  return snapshot_matches, status_changed, actual

def test_all(ci_mode=False, verbose=False):
  """Test all fixtures"""
  Path(SNAPSHOTS_DIR).mkdir(exist_ok=True)
  fixtures = list(Path(EXAMPLES_DIR).glob("*.js"))

  results = {
    'total': len(fixtures),
    'matching_snapshots': 0,
    'differing_snapshots': 0,
    'new_passes': 0,
    'new_failures': 0,
  }

  for fixture in fixtures:
    snapshot_matches, status_changed, result = run_test(fixture, verbose)

    if snapshot_matches:
      results['matching_snapshots'] += 1
    else:
      results['differing_snapshots'] += 1

    if status_changed:
      if result['status'] == 'pass':
        results['new_passes'] += 1
      else:
        results['new_failures'] += 1

  # Print summary
  print("---")
  print(f"Results: {results['matching_snapshots']} snapshots match, {results['differing_snapshots']} differ")

  if results['new_passes'] > 0:
    print(f"{GREEN}🎉 {results['new_passes']} test(s) now passing!{NC}")

  if results['new_failures'] > 0:
    print(f"{RED}💥 {results['new_failures']} test(s) now failing!{NC}")

  if ci_mode:
    # In CI mode, fail if any snapshot differs
    success = results['differing_snapshots'] == 0
    if not success:
      print(f"{RED}CI: Failing (snapshots differ from expected){NC}")
    else:
      print(f"{CYAN}CI: Passing (all snapshots match){NC}")
  else:
    # In normal mode, we're more permissive - just report differences
    success = results['differing_snapshots'] == 0
    if not success:
      print(f"{YELLOW}Note: Run 'make regen' to update snapshots if changes are intentional{NC}")

  return success

def regenerate_all():
  """Regenerate all snapshots"""
  Path(SNAPSHOTS_DIR).mkdir(exist_ok=True)
  print("Regenerating all snapshots...")

  fixtures = list(Path(EXAMPLES_DIR).glob("*.js"))
  pass_count = 0
  fail_count = 0

  for fixture in fixtures:
    basename = fixture.stem
    snapshot_file = Path(SNAPSHOTS_DIR) / f"{basename}.js.snap"
    print(f"Regenerating snapshot for {basename}.js...")

    result = run_program(fixture)
    save_snapshot(snapshot_file, result)

    if result['status'] == 'pass':
      pass_count += 1
    else:
      fail_count += 1

  print(f"{GREEN}All snapshots regenerated: {pass_count} passing, {fail_count} failing{NC}")

def test_specific(fixture_name, verbose=False):
  """Test a specific fixture"""
  fixture_path = Path(EXAMPLES_DIR) / f"{fixture_name}.js"

  if not fixture_path.exists():
    print(f"{RED}Error: Fixture {fixture_path} not found{NC}")
    return False

  snapshot_matches, status_changed, result = run_test(fixture_path, verbose)
  return snapshot_matches

def main():
  parser = argparse.ArgumentParser(description='Enhanced test runner for tokenizer snapshots')
  parser.add_argument('command', nargs='?', default='test',
                    help='Command to run: test, regenerate/regen, or test-<fixture>')
  parser.add_argument('--ci', action='store_true',
                    help='CI mode: only fail on new failures, not expected ones')
  parser.add_argument('-v', '--verbose', action='store_true',
                    help='Show detailed output for failing tests')

  args = parser.parse_args()

  if args.command == 'test':
    success = test_all(ci_mode=args.ci, verbose=args.verbose)
    sys.exit(0 if success else 1)

  elif args.command in ['regenerate', 'regen']:
    regenerate_all()

  elif args.command.startswith('test-'):
    fixture_name = args.command[5:]
    success = test_specific(fixture_name, verbose=args.verbose)
    sys.exit(0 if success else 1)

  else:
    print(f"Usage: {sys.argv[0]} [test|regenerate|regen|test-<fixture_name>] [--ci] [-v]")
    print()
    print("Commands:")
    print("  test                Test all fixtures (default)")
    print("  regenerate|regen    Regenerate all snapshots")
    print("  test-<name>         Test specific fixture (e.g., test-vars)")
    print()
    print("Options:")
    print("  --ci                CI mode (only fail on new failures)")
    print("  -v, --verbose       Show detailed diff output")
    print()
    print("Available fixtures:")
    for fixture in Path(EXAMPLES_DIR).glob("*.js"):
      print(f"  - {fixture.stem}")
    sys.exit(1)

if __name__ == "__main__":
    main()
