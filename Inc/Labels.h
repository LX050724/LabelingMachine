#ifndef LABELS_H
#define LABELS_H

#include <QString>
#include <QVector>

class Labels
{
protected:
    friend class classeditor;
    QVector<QString> labels;
public:
    Labels()= default;
    Labels(const QVector<QString>& _labels): labels(_labels){}
    Labels(const Labels &Copy) : labels(Copy.labels) {}

    inline const QVector<QString>& getLabels() const {
        return labels;
    }

    inline const QString operator[] (int ID) const {
        if(ID >= labels.size())
            return QString();
        return labels[ID];
    }

    inline int operator[] (const QString& name) const {
        return labels.indexOf(name);
    }

    inline int LabelCount() {
        return labels.size();
    }

    inline void addLabel(const QString& name, int ID = -1) {
        if(ID >= 0)
            labels.insert(ID, name);
        else
            labels.push_back(name);
    }

    inline void deleteLabel(const int ID) {
        if(ID >= 0)
            labels.erase(labels.begin() + ID);
    }

    inline void deleteLabel(const QString& name) {
        deleteLabel(this->operator[](name));
    }
};

#endif // LABELS_H
