#ifndef NOTECARD_H
#define NOTECARD_H
#include <QWidget>
#include "../database/database.h"

class QLabel;
class QPushButton;

class NoteCard : public QWidget
{
    Q_OBJECT
public:
    explicit NoteCard(const NoteRecord &note, QWidget *parent = nullptr);
    int noteId() const { return m_noteId; }
    void updateContent(const NoteRecord &note);

signals:
    void clicked(int noteId);
    void summarizeRequested(int noteId);
    void deleteRequested(int noteId);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    int m_noteId;
    QLabel *m_titleLabel;
    QLabel *m_descriptionLabel;
    QLabel *m_timestampLabel;
    QPushButton *m_summarizeButton;
    QPushButton *m_deleteButton;
};
#endif