#pragma once

#include <QtWidgets/QWidget>
#include "ui_QtTimeAxis.h"

class QtTimeAxis : public QWidget
{
    Q_OBJECT
public:
    QtTimeAxis(QWidget *parent = Q_NULLPTR);
	~QtTimeAxis();
private:
	void initUI();
protected:
	virtual void resizeEvent(QResizeEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void wheelEvent(QWheelEvent *e);
private:
    Ui::QtTimeAxisClass ui;
	class PrivateData;
	PrivateData *d = NULL;
};
