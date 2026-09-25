import json
from pathlib import Path
import shutil

import pytest

import cppcheckdata
from addons.misra import MisraChecker, MisraSettings, get_args_parser
from .util import dump_create, dump_remove


@pytest.fixture
def checker():
    return MisraChecker(MisraSettings(get_args_parser().parse_args([])))


@pytest.fixture
def redeclarations(tmp_path):
    source_dir = Path(__file__).parent / 'misra'
    stem = 'misra-regression-redeclarations'
    for suffix in ('.h', '.c', '-other.c'):
        shutil.copyfile(source_dir / (stem + suffix), tmp_path / (stem + suffix))
    configurations = []
    for suffix in ('.c', '-other.c'):
        source = str(tmp_path / (stem + suffix))
        dump_create(source)
        try:
            configurations.append(cppcheckdata.CppcheckData(source + '.dump').configurations[0])
        finally:
            dump_remove(source)
    return configurations


@pytest.mark.parametrize('with_declarations', [False, True])
def test_dump_variable_declarations(tmp_path, with_declarations):
    declarations = '''
        <declaration nameToken="n1" typeStartToken="t1" typeEndToken="t1"
                     isExtern="true" isStatic="false" isInit="false"/>
        <declaration nameToken="n2" typeStartToken="t2" typeEndToken="t2"
                     isExtern="false" isStatic="false" isInit="true"/>
    ''' if with_declarations else ''
    variable = '''<var id="v" nameToken="n2" typeStartToken="t2" typeEndToken="t2"
                       access="Global" scope="s" isExtern="false" isStatic="false"'''
    variable += '>' + declarations + '</var>' if with_declarations else '/>'
    dump = tmp_path / 'declarations.dump'
    dump.write_text('''<dumps language="c"><rawtokens/><suppressions/><dump cfg="">
      <tokenlist>
        <token id="t1" str="int" scope="s"/>
        <token id="n1" str="x" scope="s" variable="v"/>
        <token id="t2" str="int" scope="s"/>
        <token id="n2" str="x" scope="s" variable="v"/>
        <token id="n3" str="y" scope="s" variable="w"/>
      </tokenlist>
      <scopes><scope id="s" type="Global"><varlist><var id="v"/><var id="w"/></varlist></scope></scopes>
      <variables>''' + variable + '''
        <var id="w" nameToken="n3" typeStartToken="t2" typeEndToken="t2" access="Global" scope="s"/>
      </variables></dump></dumps>''', encoding='utf-8')
    cfg = cppcheckdata.CppcheckData(str(dump)).configurations[0]
    var, other = cfg.variables
    assert cfg.scopes[0].varlist == [var, other]
    assert cfg.tokenlist[1].variable is var
    assert cfg.tokenlist[3].variable is var
    assert other.declarations == []
    if with_declarations:
        assert [d.nameToken for d in var.declarations] == [cfg.tokenlist[1], cfg.tokenlist[3]]
        assert [d.typeStartToken for d in var.declarations] == [cfg.tokenlist[0], cfg.tokenlist[2]]
        assert [d.typeEndToken for d in var.declarations] == [cfg.tokenlist[0], cfg.tokenlist[2]]
        assert [d.isExtern for d in var.declarations] == [True, False]
        assert [d.isStatic for d in var.declarations] == [False, False]
        assert [d.isInit for d in var.declarations] == [False, True]
    else:
        assert var.declarations == []
        assert var.nameToken is cfg.tokenlist[3]


def test_redeclaration_rules(checker, monkeypatch, redeclarations):
    errors = []
    monkeypatch.setattr(checker, 'reportError', lambda token, major, minor: errors.append((major, minor, token.str)))
    cfg = redeclarations[0]
    variables = {var.nameToken.str: var for var in cfg.variables}
    assert len(variables['internal'].declarations) == 3
    assert variables['internal'].isStatic
    assert not variables['internal'].isExtern
    for name in ('prior_extern', 'tentative', 'internal', 'incomplete'):
        var = variables[name]
        assert all(d.nameToken.variable is var for d in var.declarations)
    checker.misra_8_4(cfg)
    checker.misra_8_8(cfg)
    checker.misra_8_11(cfg)
    assert errors == [(8, 4, 'missing'), (8, 8, 'internal'), (8, 11, 'incomplete')]


def test_redeclaration_ctu_summaries(checker, monkeypatch, redeclarations, tmp_path):
    summaries = {}
    monkeypatch.setattr(cppcheckdata, 'reportSummary',
                        lambda dump, name, data: summaries.setdefault(dump, []).append({'summary': name, 'data': data}))
    for index, cfg in enumerate(redeclarations):
        current = MisraChecker(MisraSettings(get_args_parser().parse_args([])))
        current._save_ctu_summary_identifiers(str(index), cfg)
        current._save_ctu_summary_usage(str(index), cfg)

    main = {s['summary']: s['data'] for s in summaries['0']}
    external = main['MisraExternalIdentifiers']
    assert len([i for i in external if i['name'] == 'duplicate_decl' and i['decl']]) == 2
    assert len([i for i in external if i['name'] == 'duplicate_decl' and not i['decl']]) == 1
    assert len([i for i in external if i['name'] == 'declarations_only']) == 2
    assert not any(i['name'] == 'declarations_only' and not i['decl'] for i in external)
    assert len([i for i in external if i['name'] == 'initialized_extern' and i['decl']]) == 1
    assert len([i for i in external if i['name'] == 'initialized_extern' and not i['decl']]) == 1
    assert not any(i['name'] == 'internal' for i in external)
    assert len([i for i in main['MisraInternalIdentifiers'] if i['name'] == 'internal']) == 1
    usage = main['MisraUsage']
    assert len([i for i in usage if i['name'] == 'local_only']) == 1
    assert not any(i['name'] == 'declarations_only' for i in usage)
    assert not any(i['file'].endswith('.h') for i in usage)

    ctu_files = []
    for name, data in summaries.items():
        path = tmp_path / (name + '.ctu-info')
        path.write_text('\n'.join(json.dumps(item) for item in data), encoding='utf-8')
        ctu_files.append(str(path))
    errors = []
    monkeypatch.setattr(checker, 'reportError',
                        lambda location, major, minor: errors.append((major, minor, location.file, location.linenr)))
    checker.analyse_ctu_info(ctu_files)

    def location(name, configuration=0):
        var = next(v for v in redeclarations[configuration].variables if v.nameToken.str == name)
        return var.nameToken.file, var.nameToken.linenr

    duplicate = next(v for v in redeclarations[0].variables if v.nameToken.str == 'duplicate_decl')
    for declaration in duplicate.declarations[:2]:
        assert (8, 5, declaration.nameToken.file, declaration.nameToken.linenr) in errors
    assert (8, 6, *location('two_definitions')) in errors
    assert (8, 6, *location('two_definitions', 1)) in errors
    assert (8, 7, *location('local_only')) in errors
    assert (8, 7, *location('elsewhere')) not in errors
    assert not any(error[:2] == (5, 9) for error in errors)
