# Creating GitHub Issues for Failing Tests

This directory contains tools to create GitHub issues for the 11 failing unit tests identified in the test suite.

## Quick Start

To create all 11 GitHub issues at once:

```bash
./create_issues.sh
```

## Prerequisites

You need the GitHub CLI (`gh`) installed and authenticated:

```bash
# Install gh CLI if not already installed
# On Ubuntu/Debian:
sudo apt install gh

# On macOS:
brew install gh

# Authenticate with GitHub
gh auth login
```

## What Gets Created

The script will create 11 GitHub issues with the following bugs:

### Addressing Mode Bugs (2 issues)
1. **ASLZX instruction** - Uses indirect addressing instead of zero page indexed
2. **ROLZX instruction** - Uses indirect addressing instead of zero page indexed

### Compare Logic Bugs (7 issues)
3. **CMPA instruction** - Uses addition instead of subtraction
4. **CMPIY instruction** - Incorrect carry flag logic
5. **CMPX instruction** - Incorrect carry flag logic
6. **CPXA instruction** - Uses addition instead of subtraction
7. **CPYA instruction** - Uses addition instead of subtraction
8. **CPYI instruction** - Sets flags on wrong variable
9. **CPYZ instruction** - Incorrect borrow logic

### Jump Logic Bug (1 issue)
10. **JMPI instruction** - Calculates indirect address incorrectly

### Test Design Question (1 issue)
11. **RTI instruction** - Test may need proper interrupt context

## Manual Creation

If you prefer to create issues manually or selectively, refer to `FAILING_TESTS_ISSUES.md` which contains detailed information for each issue including:

- Bug description
- Source code location
- Current vs expected implementation
- Test failure details
- Steps to reproduce

## After Creating Issues

Once the issues are created, you may want to:

1. Assign them to team members
2. Add them to a project board
3. Link them to this PR
4. Prioritize them based on severity

## Troubleshooting

If the script fails:

1. **Authentication error**: Run `gh auth login` and follow the prompts
2. **Permission error**: Ensure you have write access to the repository
3. **Label error**: The script assumes `bug`, `emulator`, and `question` labels exist

You can modify the `--label` flags in `create_issues.sh` if your repository uses different label names.
