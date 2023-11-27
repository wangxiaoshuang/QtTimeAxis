#pragma once

#include <QWidget>
#include <qgraphicsview.h>
#include <qgraphicsitem.h>
#include <qpropertyanimation.h>
#include <qparallelanimationgroup.h>
#include <sstream>
#include <Windows.h>
#include <qdatetime.h>
#include <qgraphicssceneevent.h>
#include <qdesktopwidget.h>
#include <qdatetime.h>
#include <qpushbutton.h>
#include <qlayout.h>

#pragma execution_character_set("utf-8")

class ITimeSetting
{
public:
	ITimeSetting() {};
	virtual ~ITimeSetting() {};
	virtual bool setStartTime(uint64_t startTime) = 0;
};

template <typename T>
class PropertyAnimation : public QPropertyAnimation
{
public:
	PropertyAnimation(QObject *parent = Q_NULLPTR) : QPropertyAnimation(parent) {}
	PropertyAnimation(QObject *target, const QByteArray &propertyName, QObject *parent = Q_NULLPTR) : QPropertyAnimation(target, propertyName, parent) {}
	virtual ~PropertyAnimation() {}
	virtual QVariant interpolated(const QVariant &from, const QVariant &to, qreal progress) const
	{
		T f = from.value<T>();
		T t = to.value<T>();
		return QVariant::fromValue(T(f + (t - f) * progress));
	}
};

template<typename T>
class EmptyProperty
{
public:
	EmptyProperty() {}
	~EmptyProperty() {}
	virtual void setValue(const T& v) { mValue = v; }
	virtual T value() const { return mValue; }
protected:
	T mValue = 0;
};

class QInt64Property : public QObject, public EmptyProperty<qint64>
{
	Q_OBJECT
		Q_PROPERTY(qint64 value READ value WRITE setValue NOTIFY valueChanged)
public:
	QInt64Property(QObject *parent = Q_NULLPTR) : QObject(parent) {}
	~QInt64Property() {};
signals:
	void valueChanged();
};

class QRefInt64Property : public QObject, public EmptyProperty<qint64>
{
	Q_OBJECT
		Q_PROPERTY(qint64 value READ value WRITE setValue NOTIFY valueChanged)
public:
	QRefInt64Property(uint32_t *lpProperty, QObject *parent = Q_NULLPTR) : QObject(parent), mlpProperty(lpProperty) {}
	~QRefInt64Property() {};
private:
	virtual void setValue(const qint64& v)
	{
		if (mlpProperty != NULL) *mlpProperty = (uint32_t)v;
		mValue = v;
	}
	virtual qint64 value() const
	{
		return mValue;
	}
signals:
	void valueChanged();
private:
	uint32_t* mlpProperty = NULL;
};

using QInt64PropertyAnimation = PropertyAnimation<qint64>;

class ITimeAxisControl
{
public:
	ITimeAxisControl() {};
	virtual ~ITimeAxisControl() {};
	virtual bool getCurrentTime(uint64_t &time) = 0;
	virtual void setCurrrentTime(uint64_t time) = 0;
};

class EmptyTimeAxisControl : public ITimeAxisControl
{
public:
	EmptyTimeAxisControl()
	{
		mCurrentTime = mStartTime = QDateTime::currentMSecsSinceEpoch();
	}
	virtual ~EmptyTimeAxisControl() {}
	virtual bool getCurrentTime(uint64_t &time) override
	{
		auto curtime = QDateTime::currentMSecsSinceEpoch();
		if (mStartTime == 0) 
			mStartTime = curtime;
		time = mCurrentTime + curtime - mStartTime;
		return true;
	}
	virtual void setCurrrentTime(uint64_t time) override
	{
		mStartTime = 0;
		mCurrentTime = time;
	}
private:
	uint64_t mStartTime = 0;
	uint64_t mCurrentTime = 0;
};

class QScrollButton : public QPushButton
{
	Q_OBJECT
public:
	explicit QScrollButton(QWidget *parent = Q_NULLPTR);
	void mousePressEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
signals:
	void buttonMoved(int x, int state/*-1����ߡ�0������1���ұ�*/);
private:
	bool _isLButtonDown = false;
	QPoint _beginPos;
};

class ScrollBar : public QWidget
{
	Q_OBJECT
public:
	explicit ScrollBar(QWidget *parent = Q_NULLPTR);
	~ScrollBar();
	void mousePressEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void paintEvent(QPaintEvent* event);
	void setSignColor(const QColor &color);
	uint32_t getSignPosition();
	void setStartValue(uint64_t startValue);
	void setStopValue(uint64_t stopValue);
	void setShowStartValue(uint64_t startValue);
	void setShowStopValue(uint64_t stopValue);
	void setCurrentValue(uint64_t curValue);
	void setSingleStep(uint32_t singleStep);
signals:
	void valueChanged(uint64_t startValue, uint64_t stopValue, int state/*-1����ߡ�0�м䡢1���ұ�*/);
	void moveValue(int x);
public slots:
	void onButtonMoved(int distance, int state);
	void onMoveValue(int x);
private:
	class PrivateData;
	PrivateData *dPtr = NULL;
};

struct TimeInfo;
class TimeAxis : public QWidget
{
	Q_OBJECT
public:
	TimeAxis(QWidget *parent = Q_NULLPTR);
	virtual ~TimeAxis();
	void doSpread(int distance, int offset);
	void setStartTime(uint64_t startTime);
	void setControl(const std::shared_ptr<ITimeAxisControl> &control);
	void movePixel(int pixel, int duration, const QEasingCurve &easing = QEasingCurve::OutQuad);
	// ����ʱ�䷶Χ
	bool setTimeRange(uint64_t startTime, uint64_t stopTime);
	bool setShowTimeRange(uint64_t showStartTime, uint64_t showStopTime);
	void getTimeRange(uint64_t* startTime, uint64_t* stopTime);
	void getShowTimeRange(uint64_t *showStartTime, uint64_t *showStopTime);
	void setTitle(const QString& title);
	//// ���õ�ǰ��ʱ�����ʾ��ͬ��ɫ�Ŀ�
	//void setTimeSlot(const std::multiset<compare_ptr<MSProtoRecordInfo> > &records);
	//// ���
	//void addTimeSlot(const std::multiset<compare_ptr<MSProtoRecordInfo> > &records);
	//// �������а�ʱ�����ʾ��ͬ��ɫ�Ŀ�
	//void setTotalTimeSlot(const std::multiset<compare_ptr<MSProtoRecordInfo> > &records);
	//// �������а�ʱ�����ʾ��ͬ��ɫ�Ŀ�
	//void addTotalTimeSlot(const std::multiset<compare_ptr<MSProtoRecordInfo> > &records);
	// ����ʱ������Ƿ����
	bool validTimeRangle(uint64_t startTime, uint64_t stopTime);
	// ���õ�ǰ�ľ���ʱ�䣬��ı���ʾʱ�䷶Χ��ֵ
	bool setCurrentTime(uint64_t time, bool isForceUpdateScaleLinePos = false /*�Ƿ�ǿ�Ƹ��¿̶��ߵ�λ��*/);
	// �ƶ�ʱ��̶��ߣ�����ı���ʾʱ�䷶Χ��ֵ
	bool setCurrentTimeEx(uint64_t time);
	// ����ǰʱ����ʾ�ڽ�����
	void jumpCurrentTime();
	// ƽ����������Ӧʱ���
	void smoothJumpTime(uint64_t toStartTime, uint64_t toStopTime, uint32_t time = 2000, bool interrupt = true, bool clear = true, const QEasingCurve &easing = QEasingCurve::OutExpo);
	// ����������ɫ
	void setTxtColor(const QColor &color1, const QColor &color2);
	// ���ÿ����ɫ
	void setBlockColor(const QColor &color1, const QColor &color2);
	//��ȡˮƽ���������ڵ�ˮƽ����
	QHBoxLayout* getScrollBarLayout();
	// ������ʾ��Ӧ���͵�¼�����ɫ
	void setShowColorFromEventType(uint64_t eventTypes);
	// �����Ƿ���ʾ��ʱ¼��
	void setShowColorTimeRecord(bool isShow);
	uint64_t getTimeFromX(float x);
	double getXFromTime(uint64_t time);
	bool hasTotalTimeSlot();
	QRectF proxyPaintScaleMark(QPainter &painter);	// ������ƿ̶���
	QRectF proxyPaintTitle(QPainter &painter);	// ������Ʊ���
	//virtual ErrorInfo enableSmartySearch(bool enable) override;
	//virtual ErrorInfo setSmartySearchRecords(const std::vector<std::shared_ptr<ITimeObject> > &eventHappens) override;

signals:
	void signalTimeSelected(uint64_t time);
	void signalShowTimeChanged(uint64_t starttime, uint64_t stoptime);
public slots:
	void onValueChanged(uint64_t startTime, uint64_t stopTime, int state);
private:
	void calcScrollBar();
	void init();
	void paint(QPainter *painter);
	void paintRecord(QPainter *painter);
	void moveTime(int pixel);
	void setTimePerPixel(int64_t timePerPixel);
	virtual void paintEvent(QPaintEvent *event);
	virtual void mousePressEvent(QMouseEvent *event) override;
	virtual void mouseMoveEvent(QMouseEvent *event) override;
	std::shared_ptr<TimeInfo> makeAnimationTimeInfo(const TimeInfo* lpPreTimeInfo, const TimeInfo * lpCurTimeInfo);
private:
	struct PrivateData;
	std::shared_ptr<PrivateData> d;
};