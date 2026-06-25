#include "samplemodel.h"

#include <QDebug>
#include <QRandomGenerator>

SampleModel::SampleModel(QObject *parent) : QAbstractListModel(parent)
{}

int SampleModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return _data.size();
}

QVariant SampleModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.row() < 0 || index.row() > _data.count() - 1)
        return QVariant();

    auto row = _data.at(index.row());

    switch (role) {
    case IdRole:
        return index.row();

    case NameRole:
        return row.first;

    case GradeRole:
        return row.second;
    }

    return QVariant();
}

QHash<int, QByteArray> SampleModel::roleNames() const
{
    return {
        {IdRole, "id"},
        {NameRole, "name"},
        {GradeRole, "grade"}
    };
}

void SampleModel::fillSampleData(int size)
{
    QString abs = "qwertyuiopasdfghjklzxcvbnm";
    QRandomGenerator r;
    for (auto i = 0; i < size; i++) {
        Row row;
        auto nameLen = r.bounded(3, 8);
        QString name;
        for (int c = 0; c < nameLen; ++c)
            name.append(abs.at(r.bounded(0, abs.size() - 1)));

        row.first = name;
        row.second = r.bounded(0, 20);
        _data.append(row);
    }

    qDebug() << _data.size() << "item(s) added as sample data";
    beginInsertRows(QModelIndex(), 0, _data.size() - 1);
    endInsertRows();
}
