/* -*- C++ -*-
 * Cppcheck - A tool for static C/C++ code analysis
 * Copyright (C) 2007-2026 Cppcheck team.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TRANSLATIONHANDLER_H
#define TRANSLATIONHANDLER_H

#include <QList>
#include <QObject>
#include <QString>

class QTranslator;

/// @addtogroup GUI
/// @{

/**
 * @brief Information for one translation.
 *
 */
struct TranslationInfo {
    /**
     * @brief Readable name for the translation (e.g. "English").
     *
     */
    QString mName;

    /**
     * @brief Filename for the translation.
     *
     */
    QString mFilename;

    /**
     * @brief ISO 639 language code for the translation (e.g. "en").
     *
     */
    QString mCode;
};

/**
 * @brief A class handling the available translations.
 *
 * This class contains a list of available translations. The class also keeps
 * track which translation is the currently active translation.
 *
 */
class TranslationHandler : public QObject {
    Q_OBJECT
public:
    explicit TranslationHandler(QObject *parent = nullptr);

    /**
     * @brief Get a list of available translations.
     * @return List of available translations.
     *
     */
    const QList<TranslationInfo>& getTranslations() const {
        return mTranslations;
    }

    /**
     * @brief Set active translation.
     * @param code ISO 639 language code for new selected translation.
     * @return true if succeeds, false otherwise.
     *
     */
    bool setLanguage(const QString &code);

    /**
     * @brief Get currently selected translation.
     * @return ISO 639 language code for current translation.
     *
     */
    const QString& getCurrentLanguage() const;

    /**
     * @brief Get translation suggestion for the system.
     * This function checks the current system locale and determines which of
     * the available translations is best as current translation. If none of
     * the available translations is good then it returns English ("en").
     * @return Suggested translation ISO 639 language code.
     *
     */
    QString suggestLanguage() const;

protected:

    /**
     * @brief Add new translation to list of available translations.
     * @param name Name of the translation ("English").
     * @param filename Filename of the translation.
     *
     */
    void addTranslation(const char *name, const char *filename);

    /**
     * @brief Find language in the list and return its index.
     * @param code ISO 639 language code.
     * @return Index at list, or -1 if not found.
     *
     */
    int getLanguageIndexByCode(const QString &code) const;

private:

    /**
     * @brief ISO 639 language code of the currently selected translation.
     *
     */
    QString mCurrentLanguage;

    /**
     * @brief List of available translations.
     *
     */
    QList<TranslationInfo> mTranslations;

    /**
     * @brief Translator class instance.
     *
     */
    QTranslator* mTranslator{};
};

/// @}
#endif // TRANSLATIONHANDLER_H
