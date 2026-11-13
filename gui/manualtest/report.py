"""The HTML report of a run.py run: summary, results per test file, history over the earlier runs,
the failed tests with their screenshots and all tests. One file, no external resources.

write(path, run, history) where run is what run.py saves as results.json and history is the
results.json of earlier runs of the same edition, oldest first.
"""

import html
import os

STATUSES = ('PASS', 'FAIL', 'SKIP')
ICON = {'PASS': '✓', 'FAIL': '✗', 'SKIP': '–', None: ''}
COLOR = {'PASS': 'var(--pass)', 'FAIL': 'var(--fail)', 'SKIP': 'var(--skip)'}
HISTORY_RUNS = 15      # columns in the history, including this run

CSS = """
:root {
  --ink: #1b2330; --ink-2: #4a5568; --muted: #6b7686; --line: #e2e6ec; --line-2: #cfd6df; --paper: #fff; --band: #f7f8fa;
  --accent: #2456c8; --accent-soft: #e8eefb;
  --pass: #0ca30c; --pass-bg: #e4f5e4; --pass-ink: #0b5a0b;
  --fail: #d03b3b; --fail-bg: #fbe6e6; --fail-ink: #9a1d1d;
  --skip: #fab219; --skip-bg: #fdf2d6; --skip-ink: #6e4b00;
  --none: #dfe3e9;
  --sans: system-ui, -apple-system, "Segoe UI", Ubuntu, Cantarell, Roboto, sans-serif;
  --mono: ui-monospace, "DejaVu Sans Mono", Menlo, Consolas, monospace;
  color-scheme: light;
}
* { box-sizing: border-box; }
body { margin: 0; background: var(--paper); color: var(--ink); font: 14px/1.5 var(--sans); }
main { max-width: 72rem; margin: 0 auto; padding: 1.6rem max(16px, 2.2rem) 3rem; }
h1, h2, h3 { margin: 0; text-wrap: balance; }
a { color: var(--accent); }
section { margin-top: 2rem; }
h2 { font-size: 1.02rem; font-weight: 650; margin-bottom: .2rem; }
.sub { color: var(--muted); font-size: .85rem; margin: 0 0 .8rem; }
.rhead { display: flex; gap: 1rem 2rem; align-items: flex-start; justify-content: space-between; flex-wrap: wrap; }
.rhead h1 { font-size: 1.35rem; font-weight: 650; }
.eyebrow { text-transform: uppercase; letter-spacing: .08em; font-size: .72rem; color: var(--muted); font-weight: 600; }
.verdict { border-radius: 8px; padding: .45rem .85rem; font-weight: 650; }
.verdict.FAIL { background: var(--fail-bg); color: var(--fail-ink); }
.verdict.PASS { background: var(--pass-bg); color: var(--pass-ink); }
.meta { display: grid; grid-template-columns: repeat(auto-fill, minmax(13rem, 1fr)); gap: .55rem 1.5rem; margin: 1.1rem 0 0;
        padding-top: 1rem; border-top: 1px solid var(--line); }
.meta div { min-width: 0; }
.meta dt { font-size: .75rem; color: var(--muted); }
.meta dd { margin: 0; overflow-wrap: anywhere; }
.meta dd.mono { font-family: var(--mono); font-size: .85rem; }
.dirty, .aborted { color: var(--fail-ink); }
.dirty { font-size: .75rem; margin-left: .3rem; }
.tiles { display: grid; grid-template-columns: repeat(auto-fit, minmax(10rem, 1fr)); gap: .8rem; margin-top: 1.4rem; }
.tile { border: 1px solid var(--line); border-radius: 8px; padding: .75rem .9rem; }
.tile .lbl { font-size: .8rem; color: var(--muted); display: flex; align-items: center; gap: .4rem; }
.tile .val { font-size: 1.7rem; font-weight: 650; line-height: 1.2; font-variant-numeric: tabular-nums; }
.tile .note { font-size: .78rem; color: var(--muted); }
.dot { width: .6rem; height: .6rem; border-radius: 50%; display: inline-block; flex: none; }
.d-PASS { background: var(--pass); } .d-FAIL { background: var(--fail); } .d-SKIP { background: var(--skip); }
.chip { display: inline-block; border-radius: 5px; padding: 0 .45rem; font-size: .75rem; font-weight: 650; line-height: 1.6; white-space: nowrap; }
.c-PASS { background: var(--pass-bg); color: var(--pass-ink); }
.c-FAIL { background: var(--fail-bg); color: var(--fail-ink); }
.c-SKIP { background: var(--skip-bg); color: var(--skip-ink); }
.c-CRASH { background: var(--fail-ink); color: #fff; }
.c-NEW { background: var(--accent-soft); color: var(--accent); }
.legend { display: flex; gap: 1rem; flex-wrap: wrap; font-size: .8rem; color: var(--ink-2); margin: .2rem 0 .5rem; }
.legend span { display: inline-flex; align-items: center; gap: .35rem; }
.chart { overflow-x: auto; }
.chart svg { display: block; width: 100%; height: auto; min-width: 34rem; }
.chart text { font-family: var(--sans); fill: var(--ink-2); font-size: 12px; }
.chart text.m { fill: var(--muted); font-size: 11px; }
.chart text.b { fill: var(--ink); font-weight: 600; }
.chart text.sha { font-family: var(--mono); font-size: 9.5px; fill: var(--muted); }
.chart .grid { stroke: var(--line); stroke-width: 1; }
.cols { display: grid; grid-template-columns: repeat(auto-fit, minmax(20rem, 1fr)); gap: 1.5rem 2.5rem; }
.cols > * { min-width: 0; }
.changes { display: grid; gap: .45rem; }
.changes div { display: flex; gap: .5rem; align-items: baseline; flex-wrap: wrap; }
.file { color: var(--muted); font-size: .8rem; }
.tbl { overflow-x: auto; }
table.tests { border-collapse: collapse; width: 100%; min-width: 40rem; }
.tests th { text-align: left; font-size: .75rem; color: var(--muted); font-weight: 600; padding: .35rem .6rem; border-bottom: 1px solid var(--line-2); }
.tests td { padding: .45rem .6rem; border-bottom: 1px solid var(--line); vertical-align: top; }
.tests tr.grp td { background: var(--band); font-weight: 600; font-size: .82rem; color: var(--ink-2); padding-top: .55rem; }
.tests tr.grp span { font-weight: 400; color: var(--muted); }
.tests td.t { white-space: nowrap; text-align: right; font-variant-numeric: tabular-nums; color: var(--ink-2); }
.tests td.links { white-space: nowrap; font-size: .82rem; }
.tests details { margin-top: .3rem; font-size: .82rem; }
.tests summary { color: var(--muted); cursor: pointer; }
.reason { font-family: var(--mono); font-size: .8rem; color: var(--ink-2); white-space: pre-wrap; overflow-wrap: anywhere; }
.strip { display: flex; gap: 2px; width: 11rem; margin-top: .35rem; }
.strip i { flex: 1; height: .5rem; background: var(--pass); }
.strip i:first-child { border-radius: 3px 0 0 3px; } .strip i:last-child { border-radius: 0 3px 3px 0; }
.strip i.f { background: var(--fail); } .strip i.n { background: var(--none); }
.stripnote { font-size: .75rem; color: var(--muted); }
.card { border: 1px solid var(--line); border-left: 4px solid var(--fail); border-radius: 8px; padding: .9rem 1rem; display: grid; gap: .5rem; }
.card + .card { margin-top: .8rem; }
.card.SKIP { border-left-color: var(--skip); }
.card-head { display: flex; gap: .5rem; align-items: baseline; flex-wrap: wrap; }
.card-head h3 { font-size: .98rem; }
.card-head .links { margin-left: auto; font-size: .82rem; }
.film { display: flex; gap: .6rem; overflow-x: auto; padding-bottom: .3rem; }
.shot { flex: none; width: 10.5rem; font-size: .75rem; color: var(--ink-2); text-decoration: none; }
.shot:hover b { text-decoration: underline; }
.shot img { display: block; width: 100%; aspect-ratio: 4/3; object-fit: cover; object-position: left top;
            border: 1px solid var(--line-2); border-radius: 5px; background: var(--band); }
.shot.bad img { outline: 3px solid var(--fail); outline-offset: -1px; }
.shot b { color: var(--ink); }
.film.small .shot { width: 7.5rem; }
table.hgrid { border-collapse: separate; border-spacing: 3px; }
.hgrid th.name { text-align: right; padding-right: .6rem; font-size: .82rem; font-weight: 500; white-space: nowrap; }
.hgrid th.run { font-size: .68rem; color: var(--muted); font-weight: 500; text-align: center; }
.hgrid th.run.cur { color: var(--ink); font-weight: 700; }
.hgrid td.cell { width: 1.6rem; height: 1.35rem; border-radius: 3px; text-align: center; font-size: .72rem; font-weight: 700; color: #fff; background: var(--band); }
.hgrid td.PASS { background: var(--pass); } .hgrid td.FAIL { background: var(--fail); }
.hgrid td.SKIP { background: var(--skip); color: var(--skip-ink); }
.hgrid td.cur { box-shadow: 0 0 0 2px var(--paper), 0 0 0 3.5px var(--ink); }
.hgrid td.note { font-size: .78rem; color: var(--muted); padding-left: .7rem; white-space: nowrap; }
@media (max-width: 520px) { .card-head .links { margin-left: 0; width: 100%; } }
"""

e = html.escape


def dur(s):
    s = int(round(s))
    return '%d min %d s' % (s // 60, s % 60) if s >= 60 else '%d s' % s


def chip(status):
    return '<span class="chip c-%s">%s %s</span>' % (status, ICON[status], status)


def crash_chip(r):
    return ' <span class="chip c-CRASH">CRASH</span>' if r['crash'] else ''


def count(results, status):
    return sum(1 for r in results if r['status'] == status)


def short_date(started):
    return '%d/%d' % (int(started[5:7]), int(started[8:10]))


def links(r, outdir):
    """Links to the files of a test that exist (a skipped test has none)."""
    out = [(r['slug'] + ext, text) for ext, text in (('.txt', 'steps'), ('.app.log', 'app log'), ('.png', 'screenshot'))
           if os.path.exists(os.path.join(outdir, r['slug'] + ext))]
    return ' · '.join('<a href="%s">%s</a>' % (e(f), t) for f, t in out)


def strip(r):
    """One cell per step: green passed, red failed, grey not run."""
    n, at = r.get('steps', 0), r.get('failed_step', 0)
    if not n:
        return ''
    if r['status'] == 'SKIP':
        cells, note = ['n'] * n, 'not run'
    elif r['status'] == 'FAIL' and not at and not r['crash']:
        cells, note = ['n'] * n, 'did not start'
    else:
        cells = ['' if not at or k < at else 'f' if k == at else 'n' for k in range(1, n + 1)]
        note = 'failed at step %d of %d' % (at, n) if at else '%d of %d steps' % (n, n)
    return '<div class="strip" title="%s">%s</div><div class="stripnote">%s</div>' % (
        note, ''.join('<i class="%s"></i>' % c for c in cells), note)


def hbar(x, y, w, h, r, fill, tip):
    """A horizontal bar, rounded at the data end only."""
    r = min(r, w / 2, h / 2)
    return ('<path d="M%.1f,%.1fh%.1fa%.1f,%.1f 0 0 1 %.1f,%.1fv%.1fa%.1f,%.1f 0 0 1 -%.1f,%.1fh-%.1fz" fill="%s"><title>%s</title></path>'
            % (x, y, w - r, r, r, r, r, h - 2 * r, r, r, r, r, w - r, fill, e(tip)))


def vbar(x, base, w, h, r, fill, tip, top):
    """A column segment; the top segment is rounded at the data end."""
    if not top:
        return '<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" fill="%s"><title>%s</title></rect>' % (x, base - h, w, h, fill, e(tip))
    r = min(r, w / 2, h)
    return ('<path d="M%.1f,%.1fv-%.1fa%.1f,%.1f 0 0 1 %.1f,-%.1fh%.1fa%.1f,%.1f 0 0 1 %.1f,%.1fv%.1fz" fill="%s"><title>%s</title></path>'
            % (x, base, h - r, r, r, r, r, w - 2 * r, r, r, r, r, h - r, fill, e(tip)))


def legend():
    return '<div class="legend">%s</div>' % ''.join('<span><i class="dot d-%s"></i>%s</span>' % (k, k) for k in STATUSES)


def per_file_chart(run):
    rows = []
    for f in run['files']:
        rs = [r for r in run['results'] if r['file'] == f['file']]
        if rs:
            rows.append(dict({k: count(rs, k) for k in STATUSES}, title=f['title'] or f['file'], n=len(rs)))
    big = max(r['n'] for r in rows)
    step = 1 if big <= 10 else 5 if big <= 50 else 20
    top = -(-big // step) * step
    left, width, rh, h = 210, 900, 30, 18
    bw = width - left - 70
    sx = lambda v: v / top * bw
    s = []
    for v in range(0, top + 1, step):
        s.append('<line class="grid" x1="%.1f" x2="%.1f" y1="4" y2="%d"/><text class="m" x="%.1f" y="%d" text-anchor="middle">%d</text>'
                 % (left + sx(v), left + sx(v), len(rows) * rh + 4, left + sx(v), len(rows) * rh + 20, v))
    for i, r in enumerate(rows):
        y = 8 + i * rh
        title = r['title'] if len(r['title']) <= 30 else r['title'][:29] + '…'
        s.append('<text x="%d" y="%.1f" text-anchor="end">%s<title>%s</title></text>' % (left - 10, y + h / 2 + 4, e(title), e(r['title'])))
        x = left
        segs = [k for k in STATUSES if r[k]]
        for j, k in enumerate(segs):
            last = j == len(segs) - 1
            w = sx(r[k]) - (0 if last else 2)       # a 2px gap between the segments
            tip = '%s\n%d %s of %d tests' % (r['title'], r[k], k, r['n'])
            s.append(hbar(x, y, w, h, 4, COLOR[k], tip) if last else
                     '<rect x="%.1f" y="%d" width="%.1f" height="%d" fill="%s"><title>%s</title></rect>' % (x, y, w, h, COLOR[k], e(tip)))
            x += sx(r[k])
        s.append('<text class="b" x="%.1f" y="%.1f">%d/%d</text>' % (x + 8, y + h / 2 + 4, r['PASS'], r['n']))
    return ('<div class="chart"><svg viewBox="0 0 %d %d" role="img" aria-label="Passed, failed and skipped tests per test file">%s</svg></div>'
            % (width, len(rows) * rh + 28, ''.join(s)))


def trend_chart(runs):
    width, left, base, top = 900, 34, 190, 14
    n = len(runs)
    bw = min(30, (width - left - 30) / n * 0.6)
    gap = min((width - left - 20 - n * bw) / max(n - 1, 1), 30)
    big = max(len(r['results']) for r in runs) or 1
    step = 5 if big <= 25 else 10 if big <= 60 else 50
    ymax = -(-big // step) * step
    sy = lambda v: v / ymax * (base - top)
    s = []
    for v in range(0, ymax + 1, step):
        s.append('<line class="grid" x1="%d" x2="%d" y1="%.1f" y2="%.1f"/><text class="m" x="%d" y="%.1f" text-anchor="end">%d</text>'
                 % (left, width - 10, base - sy(v), base - sy(v), left - 6, base - sy(v) + 4, v))
    for i, run in enumerate(runs):
        x, cur = left + 6 + i * (bw + gap), i == n - 1
        c = {k: count(run['results'], k) for k in STATUSES}
        tip = 'Run %s · %s%s\n%d pass · %d fail · %d skip' % (run['started'][:16].replace('T', ' '), run['sha'][:8],
                                                             ' (aborted)' if run.get('aborted') else '', c['PASS'], c['FAIL'], c['SKIP'])
        y0 = base
        segs = [k for k in STATUSES if c[k]]
        for j, k in enumerate(segs):
            last = j == len(segs) - 1
            s.append(vbar(x, y0, bw, sy(c[k]) - (0 if last else 2), 4, COLOR[k], tip, last))
            y0 -= sy(c[k])
        if c['FAIL']:
            s.append('<text class="b" x="%.1f" y="%.1f" text-anchor="middle">%d</text>' % (x + bw / 2, y0 - 6, c['FAIL']))
        s.append('<text class="%s" x="%.1f" y="%d" text-anchor="middle">%s</text>' % ('b' if cur else 'm', x + bw / 2, base + 16, short_date(run['started'])))
        s.append('<text class="sha" x="%.1f" y="%d" text-anchor="middle">%s</text>' % (x + bw / 2, base + 30, e(run['sha'][:6])))
        if cur:
            s.append('<text class="b" x="%.1f" y="%d" text-anchor="middle">this run</text>' % (x + bw / 2, base + 46))
    return ('<div class="chart"><svg viewBox="0 0 %d %d" role="img" aria-label="Passed, failed and skipped tests in the last runs">%s</svg></div>'
            % (width, base + 52, ''.join(s)))


def describe_history(h, runs):
    """What a row of the status history says: flaky, failing since, fixed since..."""
    seen = [x for x in h if x]
    last = h[-1]
    changes = sum(1 for a, b in zip(seen, seen[1:]) if a != b)
    since = len(h) - 1
    while since > 0 and h[since - 1] == last:
        since -= 1
    when = '%s %s (%s)' % (short_date(runs[since]['started']), runs[since]['started'][11:16], runs[since]['sha'][:6])
    if all(x == 'SKIP' for x in seen):
        return 'never ran'
    if changes >= 4:
        return 'flaky: %d changes' % changes
    if last == 'FAIL':
        return 'new failure' if since == len(h) - 1 else 'failing since ' + when
    if last == 'SKIP':
        return 'skipped since ' + short_date(runs[since]['started'])
    return 'fixed since ' + when if last == 'PASS' else ''


def history_sections(run, history):
    runs = history[-(HISTORY_RUNS - 1):] + [run]
    status = [{r['name']: r['status'] for r in x['results']} for x in runs]
    prev = status[-2]
    cur = run['results']
    new = [r for r in cur if r['status'] == 'FAIL' and prev.get(r['name']) != 'FAIL']
    fixed = [r for r in cur if r['status'] == 'PASS' and prev.get(r['name']) == 'FAIL']
    still = [r for r in cur if r['status'] == 'FAIL' and prev.get(r['name']) == 'FAIL']
    line = lambda r, tag: '<div>%s<b>%s</b><span class="file">%s</span></div>' % (tag, e(r['name']), e(r['file']))
    changes = ''.join([line(r, '<span class="chip c-NEW">NEW</span> ' + chip('FAIL')) for r in new] +
                      [line(r, chip('PASS')) for r in fixed] +
                      [line(r, '<span class="chip c-FAIL">✗ STILL FAILING</span>') for r in still]) or \
        '<p class="sub">No test changed between PASS and FAIL.</p>'
    p = runs[-2]
    out = ['<section><h2>The last %d runs</h2><p class="sub">%s. The number on a column is the failed tests.</p>%s%s</section>'
           % (len(runs), e(run['edition'] or 'All editions'), legend(), trend_chart(runs)),
           '<section><h2>Changed since the previous run</h2><p class="sub">Run %s · %s</p><div class="changes">%s</div></section>'
           % (e(p['started'][:16].replace('T', ' ')), e(p['sha'][:10]), changes)]
    rows = [r for r in cur if any(s.get(r['name'], 'PASS') != 'PASS' for s in status)]
    if rows:
        head = '<tr><th></th>%s<th></th></tr>' % ''.join(
            '<th class="run%s">%s</th>' % (' cur' if i == len(runs) - 1 else '', short_date(x['started'])) for i, x in enumerate(runs))
        body = []
        for r in rows:
            h = [s.get(r['name']) for s in status]
            cells = ''.join('<td class="cell %s%s" title="%s">%s</td>' % (
                x or '', ' cur' if i == len(runs) - 1 else '',
                e('%s · %s %s: %s' % (r['name'], short_date(runs[i]['started']), runs[i]['sha'][:8], x or 'not in this run')), ICON[x])
                for i, x in enumerate(h))
            body.append('<tr><th class="name">%s</th>%s<td class="note">%s</td></tr>' % (e(r['name']), cells, describe_history(h, runs)))
        out.append('<section><h2>Status history</h2><p class="sub">The tests that did not pass in every run. '
                   'The framed column is this run.</p><div class="tbl"><table class="hgrid">%s%s</table></div></section>'
                   % (head, ''.join(body)))
    return out, len(new) - len(fixed), len(new), len(fixed)


def film(r, outdir):
    """The screenshots after the last steps before the failure, and the final screenshot."""
    shots = [(s['file'], 'Step %d' % s['step'], s['label'], False) for s in r.get('shots', [])][-3:]
    if os.path.exists(os.path.join(outdir, r['slug'] + '.png')):
        at = r.get('failed_step', 0)
        shots.append((r['slug'] + '.png', 'Step %d · failed here' % at if at else 'At the end', '', True))
    return '<div class="film">%s</div>' % ''.join(
        '<a class="shot%s" href="%s"><img src="%s" alt="%s" loading="lazy"><b>%s</b><div>%s</div></a>' % (
            ' bad' if bad else '', e(f), e(f), e(title), e(title), e(label)) for f, title, label, bad in shots) if shots else ''


def problem_cards(run, outdir):
    rs = [r for r in run['results'] if r['status'] != 'PASS']
    rs.sort(key=lambda r: r['status'] != 'FAIL')
    return ''.join('<div class="card %s"><div class="card-head">%s%s<h3>%s</h3><span class="file">%s</span><span class="links">%s</span></div>'
                   '%s<div class="reason">%s</div>%s</div>' % (
                       r['status'], chip(r['status']), crash_chip(r), e(r['name']), e(r['file']), links(r, outdir),
                       strip(r) if r['status'] == 'FAIL' else '', e(r['reason']), film(r, outdir) if r['status'] == 'FAIL' else '')
                   for r in rs)


def all_tests(run, outdir):
    rows = []
    for f in run['files']:
        rs = [r for r in run['results'] if r['file'] == f['file']]
        if not rs:
            continue
        rows.append('<tr class="grp"><td colspan="4">%s <span>· %s · %d of %d passed</span></td></tr>' % (
            e(f['title'] or f['file']), e(f['file']), count(rs, 'PASS'), len(rs)))
        for r in rs:
            shots = r.get('shots', [])
            more = ('<details><summary>screenshots after the steps (%d)</summary><div class="film small">%s</div></details>' % (
                len(shots), ''.join('<a class="shot" href="%s"><img src="%s" alt="" loading="lazy"><b>Step %d</b><div>%s</div></a>' % (
                    e(s['file']), e(s['file']), s['step'], e(s['label'])) for s in shots))) if shots else ''
            rows.append('<tr><td>%s%s</td><td><div>%s</div>%s%s</td><td class="t">%s</td><td class="links">%s</td></tr>' % (
                chip(r['status']), crash_chip(r), e(r['name']), strip(r), more,
                dur(r['seconds']) if r['status'] != 'SKIP' else '–', links(r, outdir)))
    return ('<div class="tbl"><table class="tests"><thead><tr><th>Result</th><th>Test</th><th style="text-align:right">Time</th>'
            '<th>Files</th></tr></thead><tbody>%s</tbody></table></div>' % ''.join(rows))


def write(path, run, history):
    outdir = os.path.dirname(path)
    rs = run['results']
    c = {k: count(rs, k) for k in STATUSES}
    ran = c['PASS'] + c['FAIL']
    total = sum(r['seconds'] for r in rs)
    crashes = sum(1 for r in rs if r['crash'])
    verdict = 'FAIL' if c['FAIL'] else 'PASS'
    sha = run['sha'][:8]
    text = ('✗ FAILED · %d of %d tests failed' % (c['FAIL'], len(rs)) if c['FAIL'] else
            '✓ PASSED · all %d tests passed' % ran if not c['SKIP'] else '✓ PASSED · %d passed, %d skipped' % (ran, c['SKIP']))
    meta = [('Date', e(run['started'][:19].replace('T', ' ')), ''),
            ('Edition', e(run['edition'] or 'not given, all tests'), ''),
            ('Duration', dur(total), ''),
            ('Git SHA', e(run['sha']) + ('<span class="dirty">+ uncommitted changes</span>' if run['dirty'] else ''), 'mono'),
            ('Branch', e(run['branch']), ''),
            ('Commit', e(run['subject']), ''),
            ('Application', e(run['app']), 'mono'),
            ('Host', e(run['host']), ''),
            ('Test files', e(', '.join(f['file'] for f in run['files'])), '')]
    if run.get('aborted'):
        meta.append(('Aborted', '<span class="aborted">the run was stopped before all tests were done</span>', ''))
    hist, delta, new, fixed = history_sections(run, history) if history else ([], 0, 0, 0)

    def tile(label, value, note, status=''):
        return '<div class="tile"><div class="lbl">%s%s</div><div class="val">%s</div><div class="note">%s</div></div>' % (
            '<span class="dot d-%s"></span>' % status if status else '', label, value, note)
    tiles = [tile('Passed', c['PASS'], '%d %% of the tests' % round(100 * c['PASS'] / len(rs)) if rs else '', 'PASS'),
             tile('Failed', c['FAIL'], '%d of them crashed the app' % crashes if crashes else 'no crashes', 'FAIL'),
             tile('Skipped', c['SKIP'], 'no script or outdated script' if c['SKIP'] else '&nbsp;', 'SKIP'),
             tile('Duration', dur(total), 'average %d s per test' % round(total / ran) if ran else '&nbsp;')]
    if history:
        tiles.append(tile('Compared to the previous run', '%+d' % delta if delta else '±0',
                          '%d new failure%s, %d fixed' % (new, '' if new == 1 else 's', fixed)))

    out = ['<!DOCTYPE html>', '<html lang="en"><head><meta charset="utf-8">',
           '<meta name="viewport" content="width=device-width, initial-scale=1">',
           '<title>GUI test report %s %s</title>' % (e(run['edition'] or ''), e(sha)), '<style>' + CSS + '</style></head><body><main>',
           '<div class="rhead"><div><div class="eyebrow">GUI test report</div><h1>%s %s · %s</h1></div><span class="verdict %s">%s</span></div>' % (
               e(os.path.basename(run['app'])), e(run['edition'] or ''), e(sha), verdict, text),
           '<dl class="meta">%s</dl>' % ''.join('<div><dt>%s</dt><dd class="%s">%s</dd></div>' % (k, cls, v) for k, v, cls in meta),
           '<div class="tiles">%s</div>' % ''.join(tiles)]
    if len([f for f in run['files'] if any(r['file'] == f['file'] for r in rs)]) > 1:
        out.append('<section><h2>Results per test file</h2><p class="sub">Which part of the GUI has problems.</p>%s%s</section>'
                   % (legend(), per_file_chart(run)))
    out += hist
    if c['FAIL'] or c['SKIP']:
        out.append('<section><h2>Failed and skipped tests</h2><p class="sub">The reason, how far the test got, and the screenshots '
                   'after the last steps. The red frame is the screen when the test failed.</p>%s</section>' % problem_cards(run, outdir))
    if rs:
        out.append('<section><h2>All tests</h2><p class="sub">Grouped by test file. The strip shows the steps: green passed, '
                   'red failed, grey not run.</p>%s</section>' % all_tests(run, outdir))
    if not history:
        out.append('<p class="sub" style="margin-top:2rem">No earlier runs of this edition with results.json were found in %s, '
                   'the history is shown from the next run.</p>' % e(os.path.dirname(outdir)))
    out.append('</main></body></html>')
    with open(path, 'w', encoding='utf-8') as f:
        f.write('\n'.join(out) + '\n')
