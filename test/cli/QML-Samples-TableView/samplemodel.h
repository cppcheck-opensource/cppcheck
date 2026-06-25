#ifndef SAMPLEMODEL_H
#define SAMPLEMODEL_H

#include <QAbstractListModel>

class SampleModel : public QAbstractListModel
{
    Q_OBJECT

    typedef QPair<QString, int> Row;
    QList<Row> _data;

public:
    enum Role {
        IdRole = Qt::UserRole + 1,
        NameRole,
        GradeRole
    };

    SampleModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

public slots:
    void fillSampleData(int size);
};

#endif // SAMPLEMODEL_H
