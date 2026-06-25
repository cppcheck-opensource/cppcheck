
// Test library configuration for kde.cfg
//
// Usage:
// $ cppcheck --check-library --library=kde --library=qt --enable=style,information --inconclusive --error-exitcode=1 --inline-suppr test/cfg/kde.cpp
// =>
// No warnings about bad library configuration, unmatched suppressions, etc. exitcode=0
//

#include <cstdio>
#include <KDE/KGlobal>
#include <KDE/KConfigGroup>
#include <klocalizedstring.h>
#include <kpluginfactory.h>

class k_global_static_testclass1 {};
K_GLOBAL_STATIC(k_global_static_testclass1, k_global_static_testinstance1);

void valid_code(const KConfigGroup& cfgGroup)
{
    const k_global_static_testclass1 * pk_global_static_testclass1 = k_global_static_testinstance1;
    printf("%p", pk_global_static_testclass1);

    bool entryTest = cfgGroup.readEntry("test", false);
    if (entryTest) {}
}

void ignoredReturnValue(const KConfigGroup& cfgGroup)
{
    // cppcheck-suppress ignoredReturnValue
    cfgGroup.readEntry("test", "default");
    // cppcheck-suppress ignoredReturnValue
    cfgGroup.readEntry("test");
}

void i18n_test()
{
    (void)i18n("Text");
    (void)xi18n("Text");
    (void)ki18n("Text");
    (void)i18nc("Text", "Context");
    (void)xi18nc("Text", "Context");
    (void)ki18nc("Text", "Context");
}

class PluginWithoutMetaData : public QObject
{
    Q_OBJECT
public:
    // Add a default arg to make sure we do not get an ambiguity compiler error
    explicit PluginWithoutMetaData(const QObject *, const QVariantList &args = {})
    : QObject()
    {
        Q_UNUSED(args)
    };
};

K_PLUGIN_CLASS(PluginWithoutMetaData)

class StaticSimplePluginClass : public QObject
{
    Q_OBJECT

public:
    // Next to the assertion below, ensure that we have no ambiguity!
    explicit StaticSimplePluginClass(QObject *parent, const QString &data = {})
    : QObject(parent)
    {
        // We have added a default arg, but KPluginFactory should still provide the valid metadata instead of the default one
        data = QString("foo");
    }
};
K_PLUGIN_CLASS_WITH_JSON(StaticSimplePluginClass, "data/jsonplugin.json")

class ClipboardPlugin : public Purpose::PluginBase
{
    Q_OBJECT
public:
    using PluginBase::PluginBase;
    Purpose::Job *createJob() const override
    {
        return new ClipboardJob(nullptr);
    }
};

K_PLUGIN_FACTORY_WITH_JSON(Clipboard, "clipboardplugin.json", foo)
