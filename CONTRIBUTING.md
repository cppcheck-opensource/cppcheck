# Contributing To Cppcheck

These are some guidelines *any* contributor should follow. They will help to make for better contributions which will most likely allow for them to be processed.

(TODO: move daca@home section here?)

## Code Changes

Code contributions are handled via GitHub pull requests: https://github.com/cppcheck-opensource/cppcheck/pulls.

If you file a pull request you might not get a reply immediately. We are a very small team and it might not fit in the current scope or time.

Any kind of contribution is welcome but we might reject it. In that case we usually provide an explanation for the reasons but everything is always open to discussion.

Changes in the `externals` folder need to be submitted to the respective upstream projects and will be pulled downstream with their next stable release (`picojson` is an exception because it is no longer maintained - the handling of such changes is not yet determined - see also https://trac.cppcheck.net/ticket/12233).

Also after you filed a pull request please be ready to reply to questions and feedback. If you just "dump and leave" it might lower the chances of your change being accepted. This also applies after the PR was successfully merged as it might cause issues which were not exposed by the CI.

Please do not be discouraged if your change was rejected or if the review process might not have been as smooth as it could have been.

Each change should be accompanied with a unit ([C++](https://github.com/cppcheck-opensource/cppcheck/tree/main/test)) or integration ([Python](https://github.com/cppcheck-opensource/cppcheck/tree/main/test/cli)) test to ensure that it doesn't regress with future changes. Negative tests (testing the opposite behavior) would be favorable but might not be required or might already exist depending on the change. Tests which introduce `TODO_ASSERT_` or `@pytest.mark.skip`/`@pytest.mark.xfail` should have tickets filed.

If the change is modifying existing behavior (i.e. adding a feature or fixing a bug) it should be accompanied by an issue in the [tracker](https://trac.cppcheck.net) (if you do not have access we can assist with that). Depending on the change it might also warrant an entry in `releasenotes.txt`.

The CI is already doing a lot of work but there are certain parts it cannot ensure.

(TODO: mention test_pr.py)

The CI has an "always green" approach which means that failing tests are not allowed. Flakey tests might be acceptable depending on the frequency of their failures but they should be accompanied by ticket so they are being tracked. If introduce a test which is expected to fail you should use the `TODO_*` macros (C++) or `@pytest.mark.xfail(strict=False)` annotations.

Note: Usually you can run the CI on your own fork to verify that it passes before even open an PR. Unfortunately some changes to avoid duplicated builds in our CI disabled this (TODO: file a ticket about this).

### Targets

Cppcheck is tracking its issues at https://trac.cppcheck.net.

The tickets are not really prioritized (except for non-synthetic crashing issues) but of most interest are the following types of tickets.

[False Positives](https://trac.cppcheck.net/query?status=accepted&status=assigned&status=new&status=reopened&component=False+positive&col=id&col=summary&col=status&col=component&col=type&col=priority&col=milestone&order=priority)

Since Cppcheck aims to be low on false positives, these kind of issues are obviously of the highest priority.

[Detection Regressions](https://trac.cppcheck.net/query?status=accepted&status=assigned&status=new&status=reopened&keywords=~regression&component=Improve+check&col=id&col=summary&col=status&col=type&col=priority&col=milestone&col=component&order=priority)

Changes might lead to fewer findings being reported. In very few cases this might be intentional but we should not regress in what findings are being reported. 

[Other Defects](https://trac.cppcheck.net/query?status=accepted&status=assigned&status=new&status=reopened&type=defect&component=!False+positive&col=id&col=summary&col=type&col=status&col=component&col=priority&col=milestone&order=priority)

Note: If you start working on ticket, please assign yourself or request to be.

## Source TODOs

There are also various source-level TODOs. These might be related to already tracked issues (even if not explicitly noted) but may also be just exist exploratively, have even been added overzealously or might even be outdated.

So if you start spending a lot of time on these, you might want to get into touch before proceeding further. 

## simplecpp

At its core Cppcheck is relying on the `simplecpp` library which is a preprocessor implementation which was spun off into its [separate project](https://github.com/cppcheck-opensource/simplecpp) with its own [bug tracker](https://github.com/cppcheck-opensource/simplecpp/issues). This is also maintained by the Cppcheck developers and contributions to it are also welcome.

## Translations

We are also maintaining various translations for `cppcheck-gui`.

Several of these are not complete or might be out-of-date so contributions are welcome. We will also accept additional languages but such contributions should be complete.

(TODO: provide more details)
