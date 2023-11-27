#include "QtTimeAxis.h"
#include "TimeAxis.h"
#include <qevent.h>
struct QtTimeAxis::PrivateData
{
	int											mXOffset = 0;
	std::shared_ptr<QPoint>						mTimeMovePos;
	std::shared_ptr<QPoint>						mLButtonDownPos;
	uint64_t									mMousePressTime = 0;
	std::shared_ptr<TimeAxis>					mTimeAxis;
};

#define DEFAULT_MOVE_TIME_SCALE			2000			// 鼠标移动位置和鼠标移动时间之间的比
QtTimeAxis::QtTimeAxis(QWidget *parent)
    : QWidget(parent)
{
	setMouseTracking(true);
    ui.setupUi(this);
	d = new PrivateData();
	initUI();
}

QtTimeAxis::~QtTimeAxis()
{
	delete d;
}

void QtTimeAxis::initUI()
{
	srand(time(NULL));
	auto emptyControl = std::make_shared<EmptyTimeAxisControl>();
	d->mTimeAxis = std::make_shared<TimeAxis>(this);
	d->mTimeAxis->setControl(emptyControl);
	ui.horizontalLayout->addWidget(d->mTimeAxis.get());
}

void QtTimeAxis::resizeEvent(QResizeEvent *event)
{
}

void QtTimeAxis::mouseMoveEvent(QMouseEvent *event)
{
	d->mXOffset = event->x();
	if (d->mTimeMovePos != nullptr)
	{
		auto pos = event->pos();
		int distance = pos.x() - d->mTimeMovePos->x();
		d->mTimeAxis->movePixel(distance, 0);
		*d->mTimeMovePos = pos;
	}
}

void QtTimeAxis::mousePressEvent(QMouseEvent *event)
{
	d->mTimeMovePos = std::make_shared<QPoint>(event->pos());
	d->mLButtonDownPos = std::make_shared<QPoint>(event->pos());
	d->mMousePressTime = QDateTime::currentMSecsSinceEpoch();
}

void QtTimeAxis::mouseReleaseEvent(QMouseEvent *event)
{
	if (d->mLButtonDownPos != nullptr)
	{
		int diffx = event->x() - d->mLButtonDownPos->x();
		int distance = abs(diffx);
		uint64_t curtime = QDateTime::currentMSecsSinceEpoch();
		uint64_t totaltime = curtime - d->mMousePressTime;
		if (totaltime == 0) return;
		int uint = distance * 1000 / totaltime;
		if (uint >= DEFAULT_MOVE_TIME_SCALE)
		{
			distance = distance * (uint / 1000);
			if (diffx < 0) distance *= -1;
			d->mTimeAxis->movePixel(distance, 2000);
		}
	}
	d->mTimeMovePos = nullptr;
	d->mLButtonDownPos = nullptr;
}

void QtTimeAxis::wheelEvent(QWheelEvent *e)
{
	int delta = e->delta();

	float degrees = (float)e->delta() / 60;
	int dinstance = degrees;
	d->mTimeAxis->doSpread(dinstance, d->mXOffset);
}