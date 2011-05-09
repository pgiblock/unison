#include <QObject>
#include <QTimer>
#include <iostream>

#include "qingen/World.hpp"
#include "ingen/EngineBase.hpp"

// A crappy class to whip (Q)Ingen.  Temporary.
class BackgroundStuff : public QObject
{
	Q_OBJECT
public:
	BackgroundStuff (QObject* parent, QIngen::World* world) :
		QObject(parent),
		m_world(world)
	{
		QTimer* timer = new QTimer(this);
		connect(timer, SIGNAL(timeout()), this, SLOT(fire()));
		timer->start(100);
	}

	virtual ~BackgroundStuff() {};

public slots:
	void printConnected ()
	{
		std::cout << "CONNECTED!!!!\n";
	}

	void fire ()
	{
		m_world->engine()->main_iteration();
	}

private:
	QIngen::World* m_world;
};


