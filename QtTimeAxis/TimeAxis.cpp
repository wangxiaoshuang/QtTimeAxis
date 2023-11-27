#include "TimeAxis.h"
#include <qgraphicsscene.h>
#include <qgraphicssceneevent.h>
#include <qgraphicsitem.h>
#include <qanimationgroup.h>
#include <qparallelanimationgroup.h>
#include <qsequentialanimationgroup.h>
#include <qpropertyanimation.h>
#include <qevent.h>
#include <qtimeline.h>
#include <set>
#include <qtooltip.h>
#include "ui_TimeAxis.h"

//static const QColor clLeval0(0x7A888F);
static const QColor clLeval1(0xA5B7C0);
static const QColor clLeval2(0x53707F);
static const QColor clLeval3(0x3F545F);
static const QColor clLeval4(0x2B383F);

enum TimeUint
{
	Year = 0,
	Month,
	Day,
	Hour,
	Minutes,
	Second,
	Millisecond,
};

enum TimeType
{
	TimeType_UTC = 0,		// 绝对时间
	TimeType_Relative,		// 相对时间
};

struct TimeStart
{
	TimeUint mTimeUint = Millisecond;
	int32_t mStartTime = 0;
};

struct TimeBase
{
	TimeBase() {};
	TimeBase(uint64_t baseTime) : mBaseTime(baseTime), mTimeType(TimeType_UTC) {}
	TimeBase(uint64_t baseTime, const std::vector<TimeStart> &startTimes) : mBaseTime(baseTime), mStartTimes(startTimes), mTimeType(TimeType_Relative) {}
	bool operator==(const TimeBase &other) const
	{
		if (mBaseTime != other.mBaseTime) return false;
		if (mTimeType != other.mTimeType) return false;
		if (mStartTimes.size() != other.mStartTimes.size()) return false;
		for (size_t i = 0; i < mStartTimes.size(); i++)
		{
			if (mStartTimes[i].mTimeUint != other.mStartTimes[i].mTimeUint) return false;
			if (mStartTimes[i].mStartTime != other.mStartTimes[i].mStartTime) return false;
		}
		return true;
	}
	bool operator!=(const TimeBase &other) const
	{
		return !(*this == other);
	}
	bool operator<(const TimeBase &other) const
	{
		return toString() < other.toString();
	}
	std::string toString() const
	{
		std::ostringstream stream;
		stream << mBaseTime << mTimeType;
		for (auto it = mStartTimes.begin(); it != mStartTimes.end(); it++)
		{
			stream << it->mStartTime << it->mTimeUint;
		}
		return stream.str();
	}
	uint64_t mBaseTime = 0;
	std::vector<TimeStart> mStartTimes;
	TimeType mTimeType = TimeType_UTC;
};

struct TimePoint
{
	bool operator==(const TimePoint &other) const
	{
		if (mBaseTime != other.mBaseTime) return false;
		if (mHeight != other.mHeight) return false;
		if (mAlpha != other.mAlpha) return false;
		return true;
	}
	TimeBase mBaseTime;
	uint64_t mHeight = 0;
	QString mTimeFormat;
	QString mTimeName;
	const QColor *mLineColor = NULL;
	int mAlpha = 255;						// 透明度
	bool mSplitPoint = false;				// 是否分割点
};

struct TimeInfo
{
	uint64_t minInteger = 0;	// 最小整数
	std::vector<TimePoint> timePoints;
};
uint32_t getLeval(uint64_t timePerPixel)
{
	if (timePerPixel == 0) return -1;
	if (timePerPixel <= 2000) return 0;
	if (timePerPixel <= 4500) return 1;
	if (timePerPixel <= 10000) return 2;
	if (timePerPixel <= 35000) return 3;
	if (timePerPixel <= 175000) return 4;
	if (timePerPixel <= 325000) return 5;
	if (timePerPixel <= 650000) return 6;
	if (timePerPixel <= 1000000) return 7;
	if (timePerPixel <= 1100000) return 8;
	if (timePerPixel <= 5300000) return 9;
	if (timePerPixel <= 10000000) return 10;
	if (timePerPixel <= 11500000) return 11;
	if (timePerPixel <= 23000000) return 12;
	if (timePerPixel <= 32000000) return 13;
	if (timePerPixel <= 45000000) return 14;
	if (timePerPixel <= 85000000) return 15;
	if (timePerPixel <= 98000000) return 16;
	if (timePerPixel <= 300000000) return 17;
	if (timePerPixel <= 550000000) return 18;
	if (timePerPixel <= 1000000000) return 19;
	if (timePerPixel <= 1500000000) return 20;
	if (timePerPixel <= 2500000000) return 21;
	if (timePerPixel <= 8000000000) return 22;
	if (timePerPixel <= 10000000000) return 23;
	if (timePerPixel <= 18000000000) return 24;
	if (timePerPixel <= 22000000000) return 25;
	return 26;
}

#define millisecond_50				50							// 50毫秒
#define millisecond_100				100							// 100毫秒
#define millisecond_500				500							// 500毫秒
#define second_1					1000						// 1秒
#define second_5					(5 * 1000)					// 5秒
#define second_10					(10 * 1000)					// 10秒
#define second_30					(30 * 1000)					// 30秒
#define minutes_1					(60 * 1000)					// 1分钟
#define minutes_5					(5 * 60 * 1000)				// 5分钟
#define minutes_10					(10 * 60 * 1000)			// 10分钟
#define minutes_30					(30 * 60 * 1000)			// 30分钟
#define hour_1						(60 * 60 * 1000)			// 1小时
#define hour_3						(3 * 60 * 60 * 1000)		// 3小时
#define hour_12						(12 * 60 * 60 * 1000)		// 12小时
#define day_1						(24 * 60 * 60 * 1000)		// 1天


static const std::map<uint32_t/*timePerPixel leval*/, TimeInfo> g_TimeInfos = {
	{0, {millisecond_50, {{minutes_1, 15, "h:mm","分", &clLeval1, 255, true}, {second_1, 15, "s", "秒", &clLeval1}, {millisecond_500, 10, "zzz", "毫秒", &clLeval2}, {millisecond_100, 5, "zzz", "毫秒", &clLeval3}, {millisecond_50, 5, "", "", &clLeval4}}}},
	{1, {millisecond_100,{{minutes_1, 15, "h:mm","分", &clLeval1, 255, true}, {second_5, 15, "s", "秒", &clLeval1}, {second_1, 15, "s", "秒", &clLeval1}, {millisecond_500, 10, "zzz", "毫秒", &clLeval2}, {millisecond_100, 5, "zzz", "毫秒", &clLeval3, 0}}}},
	{2, {millisecond_100,{{minutes_1, 15, "h:mm","分", &clLeval1, 255, true}, {second_10, 15, "s", "秒", &clLeval1}, {second_5, 15, "s", "秒", &clLeval1}, {second_1, 10, "s", "秒", &clLeval2}, {millisecond_500, 5, "zzz", "毫秒", &clLeval3}, {millisecond_100, 5, "zzz", "毫秒", &clLeval4, 0} }}},
	{3, {millisecond_500,{{minutes_1, 15, "h:mm","分", &clLeval1, 255, true}, {second_30, 15, "s", "秒", &clLeval1}, {second_10, 15, "s", "秒", &clLeval1}, {second_5, 10, "s", "秒", &clLeval2}, {second_1, 5, "s", "秒", &clLeval3}, {millisecond_500, 5, "zzz", "毫秒", &clLeval4, 0} }}},
	{4, {second_1,{{minutes_1, 15, "h:mm","分", &clLeval1, 255, true}, {second_30, 15, "s", "秒", &clLeval1}, {second_10, 10, "s", "秒", &clLeval2}, {second_5, 5, "s", "秒", &clLeval3}, {second_1, 5, "s", "秒", &clLeval4, 0} }}},
	{5, {second_5,{{hour_1, 15, "h","时", &clLeval1, 255, true}, {minutes_5, 15, "h:mm", "分", &clLeval1}, {minutes_1, 15, "h:mm", "分", &clLeval1}, {second_30, 10, "s", "秒", &clLeval2}, {second_10, 5, "s", "秒", &clLeval3}, {second_5, 5, "s", "秒", &clLeval4, 0} }}},
	{6, {second_10,{{hour_1, 15, "h","时", &clLeval1, 255, true}, {minutes_10, 15, "h:mm", "分", &clLeval1}, {minutes_5, 15, "h:mm", "分", &clLeval1}, {minutes_1, 10, "h:mm", "分", &clLeval2}, {second_30, 5, "s", "秒", &clLeval3}, {second_10, 5, "s", "秒", &clLeval4, 0} }}},
	{7, {second_30,{{hour_1, 15, "h","时", &clLeval1, 255, true}, {minutes_10, 15, "h:mm", "分", &clLeval1}, {minutes_5, 10, "h:mm", "分", &clLeval2}, {minutes_1, 5, "h:mm", "分", &clLeval3}, {second_30, 5, "s", "秒", &clLeval4, 0} }}},
	{8, {minutes_1,{{hour_1, 15, "h","时", &clLeval1, 255, true}, {minutes_30, 15, "h:mm", "分", &clLeval1}, {minutes_10, 15, "h:mm", "分", &clLeval1}, {minutes_5, 10, "h:mm", "分", &clLeval2}, {minutes_1, 5, "h:mm", "分", &clLeval3, 0} }}},
	{9, {minutes_1,{{hour_1, 15, "h","时", &clLeval1, 255, true}, {minutes_30, 15, "h:mm", "分", &clLeval1}, {minutes_10, 10, "h:mm", "分", &clLeval2}, {minutes_5, 5, "h:mm", "分", &clLeval3}, {minutes_1, 5, "h:mm", "分", &clLeval4, 0} }}},
	{10, {minutes_5,{{hour_1, 15, "h","时", &clLeval1, 255, true}, {minutes_30, 10, "h:mm", "分", &clLeval2}, {minutes_10, 5, "h:mm", "分", &clLeval3}, {minutes_5, 5, "h:mm", "分", &clLeval4, 0}}}},
	{11, {minutes_10,{ {hour_3, 15, "h","时", &clLeval1, 255, true}, {hour_1, 15, "h","时", &clLeval1, 255, true}, {minutes_30, 10, "h:mm", "分", &clLeval2}, {minutes_10, 5, "h:mm", "分", &clLeval3, 0}}}},
	{12, {minutes_10,{{hour_3, 15, "h","时", &clLeval1, 255, true}, {hour_1, 10, "h", "时", &clLeval2, 255, true}, {minutes_30, 5, "h:mm", "分", &clLeval3}, {minutes_10, 5, "h:mm", "分", &clLeval4, 0}}}},
	{13, {minutes_10,{{day_1, 15, "dd","日", &clLeval1, 255, true}, {hour_12, 15, "h","时", &clLeval1}, {hour_3, 15, "h","时", &clLeval1}, {hour_1, 10, "h", "时", &clLeval2}, {minutes_30, 5, "h:mm", "分", &clLeval3}, {minutes_10, 5, "h:mm", "分", &clLeval4, 0}}}},
	{14, {minutes_30,{{day_1, 15, "dd","日", &clLeval1, 255, true}, {hour_12, 15, "h","时", &clLeval1}, {hour_3, 15, "h","时", &clLeval1}, {hour_1, 10, "h", "时", &clLeval2}, {minutes_30, 5, "h:mm", "分", &clLeval3, 0}}}},
	{15, {minutes_30,{{day_1, 15, "dd","日", &clLeval1, 255, true}, {hour_12, 15, "h","时", &clLeval1}, {hour_3, 10, "h", "时", &clLeval2}, {hour_1, 5, "h", "时", &clLeval3}, {minutes_30, 5, "h:mm", "分", &clLeval4, 0}}}},
	{16, {hour_1,{{day_1, 15, "dd","日", &clLeval1, 255, true}, {hour_12, 15, "h","时", &clLeval1}, {hour_3, 10, "h", "时", &clLeval2}, {hour_1, 5, "h", "时", &clLeval3, 0}}}},
	{17, {hour_1,{{day_1, 15, "dd","日", &clLeval1, 255, true}, {hour_12, 10, "h","时", &clLeval2}, {hour_3, 5, "h", "时", &clLeval3}, {hour_1, 5, "h", "时", &clLeval4, 0}}}},
	{18, {hour_3,{{day_1, 15, "dd","日", &clLeval1, 255, true}, {hour_12, 10, "h","时", &clLeval2}, {hour_3, 5, "h", "时", &clLeval3, 0}}}},
	{19, {hour_3,{{ {day_1, {{Day, 1} } }, 15, "MMMM","", &clLeval1, 255, true}, {day_1, 15, "dd","日", &clLeval1}, {hour_12, 10, "h", "时", &clLeval2}, {hour_3, 5, "h", "时", &clLeval3, 0}}}},
	{20, {hour_3,{{ {day_1, {{Day, 1} } }, 15, "MMMM","", &clLeval1, 255, true}, {day_1, 15, "dd","日", &clLeval1}, {hour_12, 10, "h", "时", &clLeval2, 0}, {hour_3, 5, "h", "时", &clLeval3, 0}}} },
	{21, {hour_12,{{ {day_1, {{Day, 1} } }, 15, "MMMM","", &clLeval1, 255, true}, {day_1, 10, "dd","日", &clLeval1}, {hour_12, 5, "h", "时", &clLeval2, 0}}} },
	{22, {hour_12,{{ {day_1, {{Day, 1} } }, 15, "MMMM","", &clLeval1, 255, true}, {day_1, 10, "dd","日", &clLeval2, 0}, {hour_12, 5, "h", "时", &clLeval3, 0}}} },
	{23, {day_1,{{ {day_1, {{Day, 1} } }, 15, "yyyy","", &clLeval1, 255, true}, { {day_1, {{Day, 1} } }, 15, "MMMM","", &clLeval1}, {day_1, 10, "dd","日", &clLeval2, 0}}}},
	{24, {day_1,{{ {day_1, { {Month, 1}, {Day, 1} } }, 15, "yyyy","", &clLeval1, 255, true}, { {day_1, {{Day, 1} } }, 15, "MMMM","", &clLeval1}, {day_1, 10, "dd","日", &clLeval2, 0}}}},
	{25, {day_1,{{ {day_1, { {Month, 1}, {Day, 1} } }, 15, "yyyy","", &clLeval1, 255, true}, { {day_1, {{Day, 1} } }, 15, "MMMM","", &clLeval1}, {day_1, 0, "dd","日", &clLeval2, 0}}}},
	{26, {day_1,{{ {day_1, { {Month, 1}, {Day, 1} } }, 15, "yyyy","", &clLeval1, 255, true}, { {day_1, {{Day, 1} } }, 10, "MMMM","", &clLeval2}}}},
};

static const TimeInfo* getTimeInfo(int leval)
{
	auto it = g_TimeInfos.find(leval);
	if (it == g_TimeInfos.end()) return NULL;
	return &it->second;
}
std::map<QEasingCurve::Type, QString> g_AnimationTypes = {
{QEasingCurve::Linear, "Linear"},
{QEasingCurve::InQuad, "InQuad"},
{QEasingCurve::OutQuad, "OutQuad"},
{QEasingCurve::InOutQuad, "InOutQuad"},
{QEasingCurve::OutInQuad, "OutInQuad"},
{QEasingCurve::InCubic, "InCubic"},
{QEasingCurve::OutCubic, "OutCubic"},
{QEasingCurve::InOutCubic, "InOutCubic" },
{QEasingCurve::OutInCubic , "OutInCubic"},
{QEasingCurve::InQuart , "InQuart"},
{QEasingCurve::OutQuart, "OutQuart"},
{QEasingCurve::InOutQuart, "InOutQuart"},
{QEasingCurve::OutInQuart, "OutInQuart"},
{QEasingCurve::InQuint, "InQuint"},
{QEasingCurve::OutQuint, "OutQuint"},
{QEasingCurve::InOutQuint, "InOutQuint"},
{QEasingCurve::OutInQuint, "OutInQuint"},
{QEasingCurve::InSine,"InSine"},
{QEasingCurve::OutSine, "OutSine"},
{QEasingCurve::InOutSine, "InOutSine"},
{QEasingCurve::OutInSine, "OutInSine"},
{QEasingCurve::InExpo, "InExpo"},
{QEasingCurve::OutExpo, "OutExpo"},
{QEasingCurve::InOutExpo, "InOutExpo"},
{QEasingCurve::OutInExpo,"OutInExpo"},
{QEasingCurve::InCirc, "InCirc"},
{QEasingCurve::OutCirc, "OutCirc"},
{QEasingCurve::InOutCirc, "InOutCirc"},
{QEasingCurve::OutInCirc, "OutInCirc"},
{QEasingCurve::InElastic, "InElastic"},
{QEasingCurve::OutElastic, "OutElastic"},
{QEasingCurve::InOutElastic, "InOutElastic"},
{QEasingCurve::OutInElastic, "OutInElastic"},
{QEasingCurve::InBack, "InBack"},
{QEasingCurve::OutBack, "OutBack"},
{QEasingCurve::InOutBack, "InOutBack"},
{QEasingCurve::OutInBack, "OutInBack"},
{QEasingCurve::InBounce, "InBounce"},
{QEasingCurve::OutBounce, "OutBounce"},
{QEasingCurve::InOutBounce, "InOutBounce"},
{QEasingCurve::OutInBounce, "OutInBounce"},
{QEasingCurve::InCurve, "InCurve"},
{QEasingCurve::OutCurve, "OutCurve"},
{QEasingCurve::SineCurve, "SineCurve"},
{QEasingCurve::CosineCurve, "CosineCurve"},
{QEasingCurve::BezierSpline, "BezierSpline"},
{QEasingCurve::TCBSpline, "TCBSpline"},
{QEasingCurve::Custom, "Custom"},
{QEasingCurve::NCurveTypes, "NCurveTypes"}
};

QScrollButton::QScrollButton(QWidget *parent)
	: QPushButton(parent)
{
	setMouseTracking(true);
}

void QScrollButton::mousePressEvent(QMouseEvent *e)
{
	if (e->button() == Qt::LeftButton)
	{
		_isLButtonDown = true;
		_beginPos = e->pos();
		mapToParent(_beginPos);
	}
}

void QScrollButton::mouseReleaseEvent(QMouseEvent *e)
{
	if (e->button() == Qt::LeftButton) _isLButtonDown = false;
}

void QScrollButton::mouseMoveEvent(QMouseEvent *e)
{
	if (!_isLButtonDown)
	{
		__super::mouseMoveEvent(e);
		return;
	}
	QPoint pt = e->pos();
	int mouse_offset = pt.x();
	mapToParent(pt);
	int distance_x = pt.x() - _beginPos.x();
	QPoint ptParent = mapToParent(pt);
	if (ptParent.x() - mouse_offset <= 0 && distance_x < 0 || ptParent.x() <= 0)
	{
		// 限制移动到最左边
		emit buttonMoved(0, -1);
		return;
	}
	int parentWidth = ((QWidget*)parent())->width();
	if (ptParent.x() + (width() - mouse_offset) >= parentWidth && distance_x > 0 || ptParent.x() >= parentWidth)
	{
		// 限制移动到最右边
		emit buttonMoved(0, 1);
		return;
	}
	if (distance_x == 0) return;
	emit buttonMoved(distance_x, 0);
	__super::mouseMoveEvent(e);
}


class ScrollBar::PrivateData
{
public:
	QColor mSignColor = QColor(225, 231, 234);
	uint32_t mSignPos = 0;
	QScrollButton *mPushButton = NULL;
	uint64_t mStartValue = 0;
	uint64_t mStopValue = 0;
	uint64_t mStartShowValue = 0;
	uint64_t mStopShowValue = 0;
	uint64_t mCurrentValue = 0;
	uint32_t mSingleStep = 1;
	bool mIsScaleButton = false;
	bool mLButtonDown = false;
};

ScrollBar::ScrollBar(QWidget *parent)
	: QWidget(parent)
	, dPtr(new PrivateData())
{
	setMinimumHeight(20);
	setAttribute(Qt::WA_StyledBackground, true);
	setStyleSheet("* { background-color: rgba(46, 46, 46, 100) }");
	dPtr->mPushButton = new QScrollButton(this);
	dPtr->mPushButton->setObjectName(QStringLiteral("QScroll_Button"));
	dPtr->mPushButton->setStyleSheet("QPushButton{background-color: rgba(68, 91, 103, 100)}"
		"QPushButton:hover{ background-color: rgba(68, 91, 103, 150) }");
	dPtr->mPushButton->setCursor(Qt::PointingHandCursor);
	connect(dPtr->mPushButton, &QScrollButton::buttonMoved, this, &ScrollBar::onButtonMoved);
	connect(this, &ScrollBar::moveValue, this, &ScrollBar::onMoveValue, Qt::QueuedConnection);
	setMouseTracking(true);
}

ScrollBar::~ScrollBar()
{
	delete dPtr;
	dPtr = NULL;
}

void ScrollBar::onButtonMoved(int distance, int state)
{
	if (state != 0)
	{
		emit valueChanged(0, 0, state);
		return;
	}
	uint64_t total = dPtr->mStopValue - dPtr->mStartValue;
	double time_w = (double)total / width();			// 单位宽度的时间
	int64_t offsettime = abs(distance) * time_w;
	uint64_t leftDistance = abs((int64_t)(dPtr->mStartValue - dPtr->mStartShowValue));
	uint64_t rightDistance = abs((int64_t)(dPtr->mStopValue - dPtr->mStopShowValue));
	uint64_t stime = 0, etime = 0;
	if (distance < 0)
	{
		if (leftDistance < offsettime) offsettime = leftDistance;
		stime = dPtr->mStartShowValue - offsettime;
		etime = dPtr->mStopShowValue - offsettime;
	}
	else
	{
		if (rightDistance < offsettime) offsettime = rightDistance;
		stime = dPtr->mStartShowValue + offsettime;
		etime = dPtr->mStopShowValue + offsettime;
	}
	if (dPtr->mIsScaleButton)
	{
		uint64_t startvalue = dPtr->mStartShowValue;
		uint64_t stopvalue = dPtr->mStartShowValue + time_w * 30;
		if (dPtr->mCurrentValue > startvalue && dPtr->mCurrentValue < stopvalue)
		{
			total = dPtr->mStopShowValue - dPtr->mStartShowValue;
			uint64_t starttime = dPtr->mCurrentValue - total / 2;
			uint64_t stoptime = starttime + total;
			if (stoptime > dPtr->mStopValue)
			{
				stoptime = dPtr->mStopValue;
				starttime = stoptime - total;
			}
			stime = starttime;
			etime = stoptime;
		}
	}
	emit valueChanged(stime, etime, 0);
}

void ScrollBar::onMoveValue(int x)
{
	uint64_t total = dPtr->mStopValue - dPtr->mStartValue;
	double time_w = (double)total / width();
	int btn_x = dPtr->mPushButton->x();
	int btn_w = dPtr->mPushButton->width();
	uint32_t time = dPtr->mSingleStep * time_w;
	if (x == btn_x) return;
	if (x < btn_x)
	{
		dPtr->mStartShowValue -= time;
		dPtr->mStopShowValue -= time;
	}
	if (x > btn_x + btn_w)
	{
		dPtr->mStartShowValue += time;
		dPtr->mStopShowValue += time;
	}
	if (dPtr->mLButtonDown) emit moveValue(x);
	emit valueChanged(dPtr->mStartShowValue, dPtr->mStopShowValue, 0);
}

void ScrollBar::mousePressEvent(QMouseEvent *e)
{
	dPtr->mLButtonDown = true;
	emit moveValue(e->x());
}

void ScrollBar::mouseReleaseEvent(QMouseEvent *e)
{
	dPtr->mLButtonDown = false;
}

void ScrollBar::mouseMoveEvent(QMouseEvent *e)
{
	QRect rt = rect();
	rt.setX(dPtr->mSignPos);
	rt.setWidth(2);
	QPoint pt = e->pos();
	if (rt.contains(pt))
	{
		QString tips = QObject::tr("Indicator of current playing time");
		QToolTip::showText(QCursor::pos(), tips, this);
	}
}

void ScrollBar::paintEvent(QPaintEvent* event)
{
	dPtr->mIsScaleButton = false;
	int offsetx = 0;
	{
		uint64_t total = dPtr->mStopValue - dPtr->mStartValue;
		int x = 0;
		int w = width();
		if (total > 0)
		{
			double width_t = (double)width() / total;				// 单位时间的宽度
			uint64_t startoffset = dPtr->mStartShowValue - dPtr->mStartValue;
			uint64_t stopoffset = dPtr->mStopShowValue - dPtr->mStartValue;
			x = width_t * startoffset + 0.5f;
			offsetx = x;
			w = width_t * stopoffset - x + 0.5f;
			if (w < 25) w = 25;
			if (x + w > width()) x = width() - w;
		}
		dPtr->mPushButton->setGeometry(x, 0, w, height());
	}
	{
		QPainter painter(this);
		int x = 0;			// 滚动条左右预留位置
		int w = width() - x * 2;
		uint64_t totalTime = dPtr->mStopValue - dPtr->mStartValue;
		uint64_t offsettime = dPtr->mCurrentValue - dPtr->mStartValue;
		double ratio = (double)offsettime / totalTime;
		x += ratio * w + 0.5f;
		dPtr->mSignPos = x;
		QPen pen;
		pen.setColor(dPtr->mSignColor);
		pen.setWidth(2);
		painter.setPen(pen);
		int h = height();
		painter.drawLine(QPointF(dPtr->mSignPos, 0), QPointF(dPtr->mSignPos, h));
	}
}

void ScrollBar::setSignColor(const QColor &color)
{
	dPtr->mSignColor = color;
}

uint32_t ScrollBar::getSignPosition()
{
	return dPtr->mSignPos;
}

void ScrollBar::setStartValue(uint64_t startValue)
{
	dPtr->mStartValue = startValue;
}

void ScrollBar::setStopValue(uint64_t stopValue)
{
	dPtr->mStopValue = stopValue;
}

void ScrollBar::setShowStartValue(uint64_t startValue)
{
	dPtr->mStartShowValue = startValue;
}

void ScrollBar::setShowStopValue(uint64_t stopValue)
{
	dPtr->mStopShowValue = stopValue;
}

void ScrollBar::setCurrentValue(uint64_t curValue)
{
	dPtr->mCurrentValue = curValue;
}

void ScrollBar::setSingleStep(uint32_t singleStep)
{
	dPtr->mSingleStep = singleStep;
}

struct TimeAxis::PrivateData
{
	std::shared_ptr<QInt64PropertyAnimation>						mTimePerPixelAnimation;					// 时间轴缩放动画
	std::map<TimeBase, std::shared_ptr<QInt64PropertyAnimation> >	mHeightPropertyAnimations;				// 高度属性变化动画
	std::map<TimeBase, std::shared_ptr<QInt64PropertyAnimation> >	mAlphaPropertyAnimations;				// 透明度属性变化动画

	std::shared_ptr<QParallelAnimationGroup>	mParallelAnimationGroup;		// 并行动画组
	std::shared_ptr<QPropertyAnimation>			mTimeAnimation;					// 当前时间变化动画
	std::shared_ptr<ITimeAxisControl>			mControl;

	uint64_t									mMaxStartTime = 0;				// 最大允许的起始时间，微秒
	uint64_t									mMaxStopTime = 0;				// 最大允许的结束时间
	uint64_t									mStartTime = 0;					// 当前显示的起始时间
	uint64_t									mRealStartTime = 0;				// 真实显示的起始时间
	uint64_t									mRealStopTime = 0;				// 真实显示的结束时间
	int64_t										mTimePerPixel = 0;				// 每像素时间，单位微秒
	int											mXOffset = 0;					// 切换进制时，距离X轴左边的距离
	uint32_t									mHeight = 5;					// 刻度线基础高度
	uint32_t									mLineStartY = 54;				// 刻度线的起始高度
	uint64_t									mFirstTime = 0;					// 第一次播放时间
	uint64_t									mTime = 0;						// 当前播放时间，修正后
	uint64_t									mRealTime = 0;					// 当前真实的播放时间
	uint64_t									mCurTimeOffset = 0;				// 距离当前时间的偏移时间
	std::shared_ptr<TimeInfo>					mTimeInfo;						// 当前时间刻度绘制信息
	std::shared_ptr<TimeInfo>					mVisibleTimeInfo;				// 消失或出现刻度绘制信息
	Ui::TimeAxis								ui;
	std::shared_ptr<ScrollBar>					mHorizontalScrollBar = NULL;	// 横向滚动条

	QString										mTitle;
	int											mTimePos = 0;					// 当前刻度线的X位置
	std::shared_ptr<QFontMetrics>				mFontMetrics;
};

TimeAxis::TimeAxis(QWidget *parent) : QWidget(parent)
{
	init();
}

TimeAxis::~TimeAxis()
{
	d->mHeightPropertyAnimations.clear();
	d->mAlphaPropertyAnimations.clear();
	d->mTimePerPixelAnimation = nullptr;
	d->mParallelAnimationGroup = nullptr;
}

void TimeAxis::calcScrollBar()
{
	d->mHorizontalScrollBar->setCurrentValue(d->mTime);
	d->mHorizontalScrollBar->setStartValue(d->mMaxStartTime);
	d->mHorizontalScrollBar->setStopValue(d->mMaxStopTime);
	d->mHorizontalScrollBar->setShowStartValue(d->mRealStartTime);
	d->mHorizontalScrollBar->setShowStopValue(d->mRealStopTime);
}

void TimeAxis::init()
{
	setAttribute(Qt::WA_StyledBackground, true);
	setStyleSheet("background-color:rgb(28,35,39);");
	setMouseTracking(true);

	d = std::make_shared<PrivateData>();
	d->ui.setupUi(this);

	d->mHorizontalScrollBar = std::make_shared<ScrollBar>(this);
	d->mHorizontalScrollBar->setObjectName(QStringLiteral("mHorizontalScrollBar"));
	d->mHorizontalScrollBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	d->mHorizontalScrollBar->setMaximumSize(QSize(16777215, 16777215));
	d->ui.horizontalLayout_scroll->addWidget(d->mHorizontalScrollBar.get());
	connect(d->mHorizontalScrollBar.get(), &ScrollBar::valueChanged, this, &TimeAxis::onValueChanged);

	QInt64Property *timePerPixelProperty = new QInt64Property(this);
	d->mTimePerPixelAnimation = std::make_shared<QInt64PropertyAnimation>(timePerPixelProperty, "value", this);
	d->mTimePerPixelAnimation->setDuration(3000);
	d->mTimePerPixelAnimation->setEasingCurve(QEasingCurve::OutSine);
	connect(d->mTimePerPixelAnimation.get(), &QPropertyAnimation::valueChanged, [=](const QVariant &var) {
		if (d->mTimePerPixelAnimation->state() != QPropertyAnimation::Running) return;
		setTimePerPixel(var.toLongLong());
		});
	d->mParallelAnimationGroup = std::make_shared<QParallelAnimationGroup>();

	QInt64Property *timeProperty = new QInt64Property(this);
	d->mTimeAnimation = std::make_shared<QInt64PropertyAnimation>(timeProperty, "value", this);
	connect(d->mTimeAnimation.get(), &QInt64PropertyAnimation::valueChanged, [=](const QVariant &var) {
		update();
		});
	d->mTimeAnimation->setDuration(1000);
	d->mTimeAnimation->setLoopCount(-1);
	d->mTimeAnimation->setKeyValues({ { 0, 0 }, { 1, 1000 } });
	d->mTimeAnimation->start();
	
	auto curdatetime = QDateTime::currentDateTime();
	d->mMaxStartTime = curdatetime.toMSecsSinceEpoch() * 1000;
	d->mStartTime = curdatetime.toMSecsSinceEpoch() * 1000;
	d->mMaxStopTime = curdatetime.addYears(1).toMSecsSinceEpoch() * 1000;
	setTimePerPixel(2000);
}

void TimeAxis::movePixel(int pixel, int duration, const QEasingCurve &easing)
{
	if (duration == 0)
	{
		moveTime(pixel);
		return;
	}
	uint64_t startTime = d->mStartTime - pixel * d->mTimePerPixel;
	if (startTime < 0) startTime = 0;
}

bool TimeAxis::setTimeRange(uint64_t startTime, uint64_t stopTime)
{
	if (stopTime <= startTime) return false;
	d->mMaxStartTime = startTime;
	d->mMaxStopTime = stopTime;
	return true;
}

bool TimeAxis::setShowTimeRange(uint64_t showStartTime, uint64_t showStopTime)
{
	if (showStopTime <= showStartTime) return false;
	d->mStartTime = showStartTime;
	d->mTimePerPixel = (showStopTime - showStartTime) / width();
	return true;
}

void TimeAxis::getTimeRange(uint64_t* startTime, uint64_t* stopTime)
{
	*startTime = d->mMaxStartTime;
	*stopTime = d->mMaxStopTime;
}

void TimeAxis::getShowTimeRange(uint64_t *showStartTime, uint64_t *showStopTime)
{
	*showStartTime = d->mRealStartTime;
	*showStopTime = d->mRealStopTime;
}

void TimeAxis::setTitle(const QString& title)
{
	d->mTitle = title;
}
//
//void TimeAxis::setTimeSlot(const std::multiset<compare_ptr<MSProtoRecordInfo> > &records)
//{
//
//}
//
//void TimeAxis::addTimeSlot(const std::multiset<compare_ptr<MSProtoRecordInfo> > &records)
//{
//
//}
//
//void TimeAxis::setTotalTimeSlot(const std::multiset<compare_ptr<MSProtoRecordInfo> > &records)
//{
//
//}
//
//void TimeAxis::addTotalTimeSlot(const std::multiset<compare_ptr<MSProtoRecordInfo> > &records)
//{
//
//}

bool TimeAxis::validTimeRangle(uint64_t startTime, uint64_t stopTime)
{
	return false;
}

bool TimeAxis::setCurrentTime(uint64_t time, bool isForceUpdateScaleLinePos)
{
	return false;
}

bool TimeAxis::setCurrentTimeEx(uint64_t time)
{
	return false;
}

void TimeAxis::jumpCurrentTime()
{

}

void TimeAxis::smoothJumpTime(uint64_t toStartTime, uint64_t toStopTime, uint32_t time, bool interrupt, bool clear, const QEasingCurve &easing)
{

}

void TimeAxis::setTxtColor(const QColor &color1, const QColor &color2)
{

}

void TimeAxis::setBlockColor(const QColor &color1, const QColor &color2)
{

}

QHBoxLayout* TimeAxis::getScrollBarLayout()
{
	return d->ui.horizontalLayout_scroll;
}

void TimeAxis::setShowColorFromEventType(uint64_t eventTypes)
{

}

void TimeAxis::setShowColorTimeRecord(bool isShow)
{

}

uint64_t TimeAxis::getTimeFromX(float x)
{
	return d->mRealStartTime + x * d->mTimePerPixel;
}

double TimeAxis::getXFromTime(uint64_t time)
{
	return (time - d->mRealStartTime) / d->mTimePerPixel;
}

bool TimeAxis::hasTotalTimeSlot()
{
	return false;
}

QRectF TimeAxis::proxyPaintScaleMark(QPainter &painter)
{
	QRectF rt;
	int x = d->mTimePos;
	if (x && x <= width())
	{
		painter.fillRect(x, d->mLineStartY - 38, 2, 38, clLeval1);
		auto datetime = QDateTime::fromMSecsSinceEpoch(d->mRealTime);
		QString timeString = datetime.toString("yyyy-MM-dd hh:mm:ss.zzz");
		auto w = d->mFontMetrics->width(timeString);
		painter.drawText(x - w / 2 + 6, 16 + d->mFontMetrics->height(), timeString);
		rt.setX(x);
		rt.setWidth(w);
		rt.setY(15);
		rt.setHeight(53);
	}
	return rt;
}

QRectF TimeAxis::proxyPaintTitle(QPainter &painter)
{
	QRectF rt;
	int sortline_start_y = 38;
	if (!d->mTitle.isEmpty())
	{
		painter.setPen(clLeval1);
		QFontMetrics fm = painter.fontMetrics();
		int w = fm.width(d->mTitle) + 5;
		painter.fillRect(0, sortline_start_y + 15, w, 20, QColor(0, 0, 0, 0x77));
		painter.drawText(0, 65, d->mTitle);
		rt.setX(0);
		rt.setWidth(w);
		rt.setY(15);
		rt.setHeight(sortline_start_y + 15);
	}
	return rt;
}

//ErrorInfo TimeAxis::enableSmartySearch(bool enable)
//{
//
//}
//
//ErrorInfo TimeAxis::setSmartySearchRecords(const std::vector<std::shared_ptr<ITimeObject> > &eventHappens)
//{
//
//}


void TimeAxis::onValueChanged(uint64_t startTime, uint64_t stopTime, int state)
{
	if (state == -1) startTime = d->mMaxStartTime;
	if (state == 1) startTime = d->mMaxStopTime - width() * d->mTimePerPixel;
	d->mStartTime = startTime + d->mXOffset * d->mTimePerPixel;
}

void TimeAxis::moveTime(int pixel)
{
	d->mStartTime = d->mStartTime - pixel * d->mTimePerPixel;
	if (d->mStartTime < 0) d->mStartTime = 0;
}

std::shared_ptr<TimeInfo> TimeAxis::makeAnimationTimeInfo(const TimeInfo* lpPreTimeInfo, const TimeInfo * lpCurTimeInfo)
{
	std::shared_ptr<TimeInfo> timeInfo = std::make_shared<TimeInfo>(*lpCurTimeInfo);
	if (lpPreTimeInfo == NULL) return timeInfo;
	d->mParallelAnimationGroup->stop();
	d->mAlphaPropertyAnimations.clear();
	d->mHeightPropertyAnimations.clear();

	auto makeAnimationTimePoint = [&](const TimePoint &pre, const TimePoint &cur)
	{
		if (cur.mHeight != pre.mHeight)
		{
			auto heightProperty = new QRefInt64Property((uint32_t*)&cur.mHeight);
			std::shared_ptr<PropertyAnimation<qint64> > heightAnimation = std::make_shared<PropertyAnimation<qint64> >(heightProperty, "value", this);
			heightAnimation->setStartValue(pre.mHeight);
			heightAnimation->setEndValue(cur.mHeight);
			heightAnimation->setDuration(1000);
			heightAnimation->setEasingCurve(QEasingCurve::OutQuint);
			d->mParallelAnimationGroup->addAnimation(heightAnimation.get());
			d->mHeightPropertyAnimations.emplace(cur.mBaseTime, heightAnimation);
		}
		if (cur.mAlpha != pre.mAlpha)
		{
			auto alphaProperty = new QRefInt64Property((uint32_t*)&cur.mAlpha);
			std::shared_ptr<PropertyAnimation<qint64> > alphaAnimation = std::make_shared<PropertyAnimation<qint64> >(alphaProperty, "value", this);
			alphaAnimation->setStartValue(pre.mAlpha);
			alphaAnimation->setEndValue(cur.mAlpha);
			alphaAnimation->setDuration(1000);
			alphaAnimation->setEasingCurve(QEasingCurve::OutQuint);
			d->mParallelAnimationGroup->addAnimation(alphaAnimation.get());
			d->mAlphaPropertyAnimations.emplace(cur.mBaseTime, alphaAnimation);
		}
	};

	// 查找相等关系
	for (auto it = lpPreTimeInfo->timePoints.begin(); it != lpPreTimeInfo->timePoints.end(); it++)
	{
		for (auto iter = timeInfo->timePoints.begin(); iter != timeInfo->timePoints.end(); iter++)
		{
			if (iter->mBaseTime == it->mBaseTime)
			{
				makeAnimationTimePoint(*it, *iter);
				break;
			}
		}
	}
	// 需要做渐变消失动画
	auto pre = lpPreTimeInfo->timePoints.rbegin();
	auto cur = lpCurTimeInfo->timePoints.rbegin();
	uint32_t *lpHeight = NULL;
	uint32_t *lpAlpha = NULL;
	TimeBase key;
	d->mVisibleTimeInfo = nullptr;
	if (pre->mBaseTime.mBaseTime != cur->mBaseTime.mBaseTime)
	{
		if (pre->mBaseTime.mBaseTime < cur->mBaseTime.mBaseTime)
		{
			if (d->mVisibleTimeInfo == nullptr)
			{
				d->mVisibleTimeInfo = std::make_shared<TimeInfo>(pre->mBaseTime.mBaseTime < cur->mBaseTime.mBaseTime ? *lpPreTimeInfo : *lpCurTimeInfo);
				d->mVisibleTimeInfo->timePoints.clear();
			}
			d->mVisibleTimeInfo->timePoints.push_back(pre->mBaseTime.mBaseTime < cur->mBaseTime.mBaseTime ? *pre : *cur);
			auto iter = d->mVisibleTimeInfo->timePoints.rbegin();
			lpHeight = (uint32_t*)&iter->mHeight;
			lpAlpha = (uint32_t*)&iter->mAlpha;
			key = iter->mBaseTime;
		}
		else
		{
			auto iter = timeInfo->timePoints.rbegin();
			lpHeight = (uint32_t*)&iter->mHeight;
			lpAlpha = (uint32_t*)&iter->mAlpha;
			key = iter->mBaseTime;
		}
		auto heightProperty = new QRefInt64Property(lpHeight);
		std::shared_ptr<PropertyAnimation<qint64> > heightAnimation = std::make_shared<PropertyAnimation<qint64> >(heightProperty, "value", this);
		auto startValue = pre->mBaseTime.mBaseTime < cur->mBaseTime.mBaseTime ? *lpHeight : 0;
		auto stopValue = pre->mBaseTime.mBaseTime < cur->mBaseTime.mBaseTime ? 0 : *lpHeight;
		heightAnimation->setStartValue(startValue);
		heightAnimation->setEndValue(stopValue);
		heightAnimation->setDuration(1000);
		heightAnimation->setEasingCurve(QEasingCurve::OutQuint);
		d->mParallelAnimationGroup->addAnimation(heightAnimation.get());
		d->mHeightPropertyAnimations.emplace(key, heightAnimation);
		auto alphaProperty = new QRefInt64Property(lpAlpha);
		std::shared_ptr<PropertyAnimation<qint64> > alphaAnimation = std::make_shared<PropertyAnimation<qint64> >(alphaProperty, "value", this);
		auto start = pre->mBaseTime.mBaseTime < cur->mBaseTime.mBaseTime ? *lpAlpha : 0;
		auto stop = pre->mBaseTime.mBaseTime < cur->mBaseTime.mBaseTime ? 0 : *lpAlpha;
		alphaAnimation->setStartValue(start);
		alphaAnimation->setEndValue(stop);
		alphaAnimation->setDuration(1000);
		alphaAnimation->setEasingCurve(QEasingCurve::OutQuint);
		d->mParallelAnimationGroup->addAnimation(alphaAnimation.get());
		d->mAlphaPropertyAnimations.emplace(key, alphaAnimation);
	}
	d->mParallelAnimationGroup->start();
	return timeInfo;
}

void TimeAxis::setTimePerPixel(int64_t timePerPixel)
{
	auto triggerAnimation = [&]() -> bool
	{
		int preLeval = getLeval(d->mTimePerPixel);
		int curLeval = getLeval(timePerPixel);
		auto lpCurTimeInfo = getTimeInfo(curLeval);
		if (lpCurTimeInfo == NULL) return false;
		auto lpPreTimeInfo = getTimeInfo(preLeval);
		if (preLeval == curLeval) return true;
		d->mTimeInfo = makeAnimationTimeInfo(lpPreTimeInfo, lpCurTimeInfo);
		d->mParallelAnimationGroup->stop();
		d->mParallelAnimationGroup->start();
		return true;
	};
	triggerAnimation();
	d->mTimePerPixel = timePerPixel;
}

void TimeAxis::doSpread(int distance, int offset)
{
	uint64_t endTimePerPixel = d->mTimePerPixel;
	if (d->mTimePerPixelAnimation->state() == QPropertyAnimation::Running)
	{
		endTimePerPixel = d->mTimePerPixelAnimation->endValue().toULongLong();
	}
	int64_t timePerPixel = endTimePerPixel - (distance * (endTimePerPixel / 20));
	int32_t duration = 1000;
	if ((int64_t)timePerPixel < 2000)
	{
		timePerPixel = 2000;
	}
	int64_t maxTimePerPixel = (d->mMaxStopTime - d->mMaxStartTime) / width();
	if (timePerPixel > maxTimePerPixel)
	{
		timePerPixel = maxTimePerPixel;
	}
	if (endTimePerPixel == timePerPixel) return;
	d->mTimePerPixelAnimation->stop();
	// 像素时间变化
	d->mTimePerPixelAnimation->setDuration(duration);
	d->mTimePerPixelAnimation->setStartValue(d->mTimePerPixel);
	d->mTimePerPixelAnimation->setEndValue(timePerPixel);
	d->mTimePerPixelAnimation->start();
	d->mStartTime = d->mStartTime - (d->mXOffset - offset) * d->mTimePerPixel + d->mCurTimeOffset;
	d->mCurTimeOffset = 0;
	d->mXOffset = offset;
	d->mFirstTime = 0;
}

void TimeAxis::setStartTime(uint64_t startTime)
{
	d->mStartTime = startTime;
}

void TimeAxis::setControl(const std::shared_ptr<ITimeAxisControl> &control)
{
	d->mControl = control;
	d->mControl->setCurrrentTime((d->mStartTime + (width() / 2) * d->mTimePerPixel) / 1000);
	//d->mTimeMoveAnimation->start();
}

void TimeAxis::paint(QPainter *painter)
{
	// 吸取上个版本经验，因为浮点数原因出现各种误差及抖动；故此版本将毫秒转换为微秒，
	// 从以前以时间为中心改为现在的以像素为中心计算偏移点，避免浮点数参与计算。
	auto control = d->mControl;
	uint64_t curtime = 0;
	control->getCurrentTime(curtime);
	d->mRealTime = curtime;
	uint64_t offsetTime = d->mXOffset * d->mTimePerPixel;
	if (d->mRealStartTime < d->mMaxStartTime || d->mRealStopTime >= d->mMaxStopTime)
	{
		d->mStartTime = d->mStartTime + d->mCurTimeOffset;
		d->mFirstTime = curtime; // 超过显示范围不移动背景，仅移动刻度线
	}
	if (d->mFirstTime == 0) d->mFirstTime = curtime;
	uint64_t curTimeOffset = (curtime - d->mFirstTime) * 1000;
	d->mCurTimeOffset = curTimeOffset = curTimeOffset / d->mTimePerPixel * d->mTimePerPixel;
	int windowWidth = width();
	uint64_t startTime = d->mStartTime - offsetTime;
	// 限制缩放范围
	auto stopTime = startTime + windowWidth * d->mTimePerPixel;
	if (stopTime > d->mMaxStopTime)
	{
		stopTime = d->mMaxStopTime;
		startTime = d->mMaxStopTime - windowWidth * d->mTimePerPixel;
		d->mStartTime = startTime + offsetTime;
	}
	if (startTime < d->mMaxStartTime)
	{
		startTime = d->mMaxStartTime;
		d->mStartTime = startTime + offsetTime;
	}
	d->mRealStartTime = startTime + curTimeOffset;
	d->mRealStopTime = d->mRealStartTime + windowWidth * d->mTimePerPixel;
	uint64_t offsetTimeZone = (uint64_t)QDateTime::currentDateTime().offsetFromUtc() * 1000 * 1000;	// 时区时间
	startTime = startTime + curTimeOffset + offsetTimeZone;
	std::map<uint64_t, int> timePoints;
	std::shared_ptr<QFont> font = std::make_shared<QFont>(painter->font());
	font->setPixelSize(11);
	painter->setFont(*font);
	std::map<int, uint64_t> blocks;
	uint64_t lastms = startTime;
	auto leval = getLeval(d->mTimePerPixel);
	// 绘制刻度线
	for (int32_t w = 0; w <= windowWidth; w++)
	{
		uint64_t ms = 0;
		auto paintText = [&](const QString &describe, int x, int height, const std::shared_ptr<QFont> &lpFont, const QColor *lpColor, int alpha)
		{
			if (describe.isEmpty()) return;
			if (alpha == 0) return;
			QColor tmpColor;
			if (alpha != 255)
			{
				tmpColor = *lpColor;
				tmpColor.setAlpha(alpha);
				lpColor = &tmpColor;
			}
			int descWidth = d->mFontMetrics->width(describe);
			int descLeft = x - descWidth / 2;
			int descRight = x + descWidth / 2;
			if (descRight < 0) return;
			if (lpColor != nullptr) painter->setPen(*lpColor);
			if (lpFont != nullptr) painter->setFont(*lpFont);
			painter->drawText(descLeft, d->mLineStartY - height, describe);
		};
		auto paintLine = [&](int x, int height, const QColor* lpColor)
		{
			if (x < 0 || height <= 0) return;
			if (lpColor != nullptr) painter->setPen(*lpColor);
			painter->drawLine(x, d->mLineStartY, x, d->mLineStartY - height);
		};

		uint64_t curtime = startTime + d->mTimePerPixel * w;
		auto paintTimeAxis = [&](const TimeInfo* lpTimeInfo) -> bool
		{
			int height = 0;
			int alpha = 0;
			QString describe;
			std::shared_ptr<QFont> lpFont;
			const QColor *lpTextColor = NULL;
			const QColor *lpLineColor = NULL;
			auto calc = [&](bool &isBlockBreakPoint/*是否时间块的断开点*/)
			{
				ms = curtime / 1000 / lpTimeInfo->minInteger * lpTimeInfo->minInteger;	// 变为整数倍
				{
					auto it = timePoints.find(ms);
					if (it != timePoints.end()) return;
				}
				QDateTime datetime = QDateTime::fromMSecsSinceEpoch(ms, Qt::UTC);
				auto utc = [&](const TimePoint &timePoint) -> bool
				{
					uint64_t utc = ms;
					return utc % timePoint.mBaseTime.mBaseTime == 0;
				};
				auto relativeTime = [&](const TimePoint &timePoint) -> bool
				{
					if (!utc(timePoint)) return false;
					auto isTime = [&](const TimeStart &startTime) -> bool
					{
						switch (startTime.mTimeUint)
						{
						case Year: return datetime.date().year() == startTime.mStartTime; break;
						case Month: return datetime.date().month() == startTime.mStartTime; break;
						case Day: return datetime.date().day() == startTime.mStartTime; break;
						case Hour: return datetime.time().hour() == startTime.mStartTime; break;
						case Minutes: return datetime.time().minute() == startTime.mStartTime; break;
						case Second: return datetime.time().second() == startTime.mStartTime; break;
						case Millisecond: return datetime.time().msec() == startTime.mStartTime; break;
						default: break;
						}
						return false;
					};
					auto vec = timePoint.mBaseTime.mStartTimes;
					for (auto it = vec.begin(); it != vec.end(); it++)	// 其中一个不满足，则失败
					{
						if (!isTime(*it)) return false;
					}
					return true;
				};
				auto getTimePoint = [&](const TimePoint &timePoint) -> bool
				{
					if (timePoint.mBaseTime.mTimeType == TimeType_UTC) return utc(timePoint);
					return relativeTime(timePoint);
				};
				const TimePoint* lpTimePoint = NULL;
				for (uint32_t i = 0; i < lpTimeInfo->timePoints.size(); i++)
				{
					auto timePoint = lpTimeInfo->timePoints[i];
					if (!getTimePoint(timePoint)) continue;
					lpTimePoint = &timePoint;
					break;
				}
				if (lpTimePoint == NULL) return;
				describe = datetime.toString(lpTimePoint->mTimeFormat) + lpTimePoint->mTimeName;
				height = lpTimePoint->mHeight;
				lpTextColor = lpLineColor = lpTimePoint->mLineColor;
				alpha = lpTimePoint->mAlpha;
				isBlockBreakPoint = lpTimePoint->mSplitPoint;

				// 特殊日期，特殊对待
				auto date = datetime.date();
				auto time = datetime.time();
				if (time.minute() == 0 && time.second() == 0 && isBlockBreakPoint)
				{
					QString newDesribe;
					if (date.month() == 1 && date.day() == 1 && time.hour() == 0) newDesribe = datetime.toString("yyyy");
					else if (date.day() == 1 && time.hour() == 0) newDesribe = datetime.toString("MMMM");
					else if (time.hour() == 0) newDesribe = datetime.toString("dd日");
					else if (time.minute() == 0 && lpTimeInfo->minInteger < hour_1) newDesribe = datetime.toString("h时");
					if (newDesribe != describe)
					{
						height = 15;
						lpTextColor = lpLineColor = &clLeval1;
						describe = newDesribe;
					}
				}
			};
			bool isBlockPoint = false;
			calc(isBlockPoint);
			uint64_t us = ms * 1000;
			int x = ((double)((int64_t)(us - startTime)) / d->mTimePerPixel);
			if (isBlockPoint) blocks.emplace(x, lastms);
			paintLine(x, height, lpLineColor);
			paintText(describe, x, height + 5, lpFont, lpTextColor, alpha);
			timePoints[ms] = x;
			if (x >= width())
			{
				blocks.emplace(x, lastms);
				return false;
			}
			if (w >= windowWidth)
			{
				blocks.emplace(w, lastms);
				return false;
			}
			lastms = ms;
			return true;
		};
		if (!paintTimeAxis(d->mTimeInfo.get()))	break;
		if (d->mVisibleTimeInfo != nullptr) paintTimeAxis(d->mVisibleTimeInfo.get());
	}
	// 绘制顶部时间块
	int lastX = 0;
	for (auto it = blocks.begin(); it != blocks.end(); it++)
	{
		static const QColor color1(43, 56, 63);
		static const QColor color2(53, 70, 79);
		const QColor *lpColor = &color1;
		QDateTime datetime = QDateTime::fromMSecsSinceEpoch(it->second, Qt::UTC);
		//datetime.setTimeSpec(Qt::UTC);
		QString describe;
		auto minutes = [&]()
		{
			int minutes = datetime.time().minute();
			if (minutes % 2 != 0) lpColor = &color2;
			describe = datetime.toString("dddd dd日 MMMM yyyy hh时 mm分 AP");
		};
		auto hour = [&]()
		{
			int hour = datetime.time().hour();
			if (hour % 2 != 0) lpColor = &color2;
			describe = datetime.toString("dddd dd日 MMMM yyyy hh时 AP");
		};
		auto day = [&]()
		{
			uint64_t day = datetime.toMSecsSinceEpoch() / day_1;
			if (day % 2 != 0) lpColor = &color2;
			describe = datetime.toString("dddd dd日 MMMM yyyy");
		};
		auto month = [&]()
		{
			int month = datetime.date().month();
			if (month % 2 != 0) lpColor = &color2;
			describe = datetime.toString("MMMM yyyy");
		};
		auto year = [&]()
		{
			int year = datetime.date().year();
			if (year % 2 != 0) lpColor = &color2;
			describe = datetime.toString("yyyy");
		};
		if (leval < 5) minutes();
		else if (leval < 13) hour();
		else if (leval < 19) day();
		else if (leval < 24) month();
		else year();
		auto paintBlock = [&](int x, int w)
		{
			painter->fillRect(x, 0, w, 16, *lpColor);
			int txt_w = d->mFontMetrics->width(describe);
			int txt_x = x;
			if (x + w < txt_w) txt_x = w - txt_w;
			if (w > txt_w) txt_x = x + (w - txt_w) / 2;
			QColor color(197, 204, 208);
			painter->setPen(color);
			painter->drawText(txt_x, d->mFontMetrics->height(), describe);
		};
		int x = it->first;
		if (x < 0) x = 0;
		if (x > windowWidth) x = windowWidth;
		int w = x - lastX;
		paintBlock(lastX, w);
		lastX = x;
	}
	calcScrollBar();
	// 绘制当前时间线
	auto startms = startTime - offsetTimeZone;
	auto timeperPixel = d->mTimePerPixel;
	d->mTime = curTimeOffset + d->mFirstTime * 1000;
	d->mTimePos = (int)((double)((int64_t)(d->mTime - startms)) / timeperPixel);
}

void TimeAxis::paintRecord(QPainter *painter)
{
	int nWidth = width();
	painter->fillRect(0, d->mLineStartY, nWidth, 20, QColor(34, 57, 30));
}

void TimeAxis::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	d->mFontMetrics = std::make_shared<QFontMetrics>(painter.fontMetrics());
	paint(&painter);
	paintRecord(&painter);
	proxyPaintScaleMark(painter);
}

void TimeAxis::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::RightButton)
	{
		event->ignore();
		return;
	}
	uint64_t time = d->mRealStartTime + event->x() * d->mTimePerPixel;
	time /= 1000;
	d->mStartTime = d->mStartTime + d->mCurTimeOffset;
	auto datetime = QDateTime::fromMSecsSinceEpoch(time);
	d->mControl->setCurrrentTime(time);
	d->mFirstTime = 0;
	emit signalTimeSelected(time);
}

void TimeAxis::mouseMoveEvent(QMouseEvent *event)
{
	event->ignore();
}
