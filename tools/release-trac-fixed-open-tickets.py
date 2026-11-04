#!/usr/bin/env python3
"""Check that Trac tickets referenced by commits since the last tag are closed.

Runs `git log <latest tag>..HEAD`, extracts referenced Trac ticket numbers
from each commit message (e.g. "Fix #15028", "Trac #5935", "ticket 8442"),
looks up each ticket's status on trac.cppcheck.net, and prints the full
commit message for any commit whose ticket is not closed.
"""

import argparse
import csv
import io
import re
import subprocess
import sys
import urllib.error
import urllib.request

TRAC_BASE = 'https://trac.cppcheck.net'

# Applied (in order) to the commit message with the trailing GitHub PR
# reference (e.g. "(#8851)") stripped from the subject line.
TICKET_PATTERNS = [
    re.compile(r'\btrac\s*#?\s*(\d+)\b', re.IGNORECASE),
    re.compile(r'\bticket\s*#?\s*(\d+)\b', re.IGNORECASE),
    re.compile(r'#(\d+)\b'),
]

# GitHub appends "(#NNNN)" (the PR number) to the end of squash-merged
# commit subjects; strip it so it isn't mistaken for a Trac ticket ref.
PR_SUFFIX = re.compile(r'\s*\(#\d+\)\s*$')


def latest_tag():
    r = subprocess.run(
        ['git', 'describe', '--tags', '--abbrev=0'],
        capture_output=True, text=True, check=False,
    )
    if r.returncode == 0:
        return r.stdout.strip()

    # 'git describe' requires the tag to be an ancestor of HEAD, which can
    # fail on a shallow clone. Fall back to the highest version tag.
    print('Warning: git describe failed, falling back to highest version tag '
          f'({r.stderr.strip()})', file=sys.stderr)
    r = subprocess.run(
        ['git', 'tag', '--sort=-v:refname'],
        capture_output=True, text=True, check=True,
    )
    tags = r.stdout.splitlines()
    if not tags:
        print('Error: no tags found', file=sys.stderr)
        sys.exit(1)
    return tags[0]


def get_commits(rev_range):
    out = subprocess.run(
        ['git', 'log', rev_range, '--format=%H%x1f%B%x1e'],
        capture_output=True, text=True, check=True,
    ).stdout
    commits = []
    for chunk in out.split('\x1e'):
        chunk = chunk.strip('\n')
        if not chunk:
            continue
        h, msg = chunk.split('\x1f', 1)
        commits.append((h, msg))
    return commits


def extract_ticket_numbers(message):
    # Ticket references live in the commit subject; the body can contain
    # unrelated "#N" text (e.g. stack trace frames like "#0 0x...") that
    # would otherwise be mistaken for ticket refs.
    subject = message.splitlines()[0] if message else ''
    subject = PR_SUFFIX.sub('', subject)
    found = []
    for pat in TICKET_PATTERNS:
        for m in pat.finditer(subject):
            n = m.group(1)
            if n not in found:
                found.append(n)
    return found


def ticket_status(ticket_id, cache):
    if ticket_id in cache:
        return cache[ticket_id]
    url = f'{TRAC_BASE}/ticket/{ticket_id}?format=csv'
    try:
        with urllib.request.urlopen(url, timeout=15) as resp:
            data = resp.read().decode('utf-8-sig')
    except urllib.error.URLError as e:
        print(f'Warning: failed to fetch ticket #{ticket_id}: {e}', file=sys.stderr)
        cache[ticket_id] = None
        return None
    row = next(csv.DictReader(io.StringIO(data)), None)
    status = row['status'] if row else None
    cache[ticket_id] = status
    return status


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--from-tag', help='starting tag/rev (default: latest tag)')
    parser.add_argument('--to', default='HEAD', help='ending rev (default: HEAD)')
    args = parser.parse_args()

    from_rev = args.from_tag or latest_tag()
    rev_range = f'{from_rev}..{args.to}'
    print(f'Checking commits in {rev_range}', file=sys.stderr)

    commits = get_commits(rev_range)
    cache = {}
    checked_tickets = set()
    open_commits = 0

    for h, msg in commits:
        tickets = extract_ticket_numbers(msg)
        for t in tickets:
            checked_tickets.add(t)
            status = ticket_status(t, cache)
            if status is None:
                print(f'--- Ticket #{t} status UNKNOWN (commit {h[:10]}) ---')
                print(msg.rstrip('\n'))
                print()
                open_commits += 1
            elif status != 'closed':
                print(f'--- Ticket #{t} is OPEN (status: {status}) (commit {h[:10]}) ---')
                print(msg.rstrip('\n'))
                print()
                open_commits += 1

    print(f'Checked {len(checked_tickets)} ticket(s) referenced in {len(commits)} commit(s).', file=sys.stderr)
    if open_commits:
        print(f'{open_commits} commit(s) reference a ticket that is not closed.', file=sys.stderr)
        sys.exit(1)
    print('All referenced tickets are closed.', file=sys.stderr)


if __name__ == '__main__':
    main()
