#ifndef IMAGELABEL_H
#define IMAGELABEL_H

#include <QLabel>

class ImageLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ImageLabel(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *event);

private:
    QImage image;

public slots:
    void setImage(const QImage &image);
};

#endif // IMAGELABEL_H
