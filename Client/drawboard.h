#ifndef DRAWBOARD_H
#define DRAWBOARD_H

#include <QWidget>
#include <QMouseEvent>
#include "../lib/packet.h"
#include "../lib/enums.h"

namespace Ui {
class DrawBoard;
}

class DrawBoard : public QWidget
{
    Q_OBJECT

public:
    explicit DrawBoard(QWidget *parent = 0);
    void giveColor(const QString &color);
    ~DrawBoard();

    QImage image();

public slots:
    void draw(const QString &color, int x1, int y1, int x2, int y2);
    void clear();
    void drawImage(QImage img);

signals:
    void sendPacket(Packet p);

private:
    Ui::DrawBoard *ui;
    QPoint lastPoint;
    bool mouseDown;
    int count;
    QString m_color;
    QImage m_image;

protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent*);
};

#endif // DRAWBOARD_H
