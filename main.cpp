#include <QCoreApplication>
#include <QHash>
#include <QVector>
#include <QDebug>
#include <QTime>
#include <QtGlobal>
#include <QPair>
#include <qmath.h>

struct Route
{
    unsigned int id;
    unsigned int firstCity;
    unsigned int secondCity;
    unsigned int distance;
    double trails;
    double lastTrails;
    bool visited;
};

struct DecisionEntry
{
    unsigned int cityId;
    double coefficient;
    bool visited;
};

namespace Controls
{
    const double alfa = 1;
    const double beta = 5;
    const double startingPheromone = 0.5f;
    const double numberOfIterations = 100;
    const unsigned int numberOfAnts = 100;
    const unsigned int startingCity = 1;
    const double eps = 1e-250;

    bool DecisionTableGetLower(const DecisionEntry &e1, const DecisionEntry &e2)
    {
        if (e1.coefficient < e2.coefficient)
            return true;
        else
            return false;
    }

    float random()
    {
        return qrand()/float(RAND_MAX);
    }

    void InitRandom()
    {
        QTime time = QTime::currentTime();
        qsrand((uint)time.msec());
    }
}

class CitiesList
{
private:
    unsigned int actualId;

public:
    QVector < Route > list;

    CitiesList() : actualId(0) {}
    ~CitiesList() {}

    bool AddRoute(unsigned int firstCity, unsigned int secondCity, unsigned int distance)
    {
        if (firstCity == secondCity || distance == 0)
            return false;

        if (firstCity > secondCity)
        {
            unsigned int tmp = firstCity;
            firstCity = secondCity;
            secondCity = tmp;
        }

        for (unsigned int i=0;i<(unsigned)list.size();++i)
        {
            if (list.at(i).firstCity == firstCity && list.at(i).secondCity == secondCity)
                return false;
        }

        Route route;
        route.id = ++actualId;
        route.firstCity = firstCity;
        route.secondCity = secondCity;
        route.distance = distance;
        route.trails = Controls::startingPheromone;
        route.lastTrails = 0;
        route.visited = false;

        list.push_back(route);
        return true;
    }

    void PrintRoutes()
    {
        if (list.size() == 0)
        {
            qDebug() << "No cities on list so no routes.\n";
            return;
        }

        for (unsigned int i=0;i<(unsigned)list.size();++i)
        {
            Route route = list.at(i);
            qDebug() << "From " << route.firstCity << " to " << route.secondCity << " is " << route.distance << " hops " << route.trails << " trails";
        }
    }

    QVector < Route > GetList()
    {
        return list;
    }

    Route *GetById(unsigned int id)
    {
        for (unsigned int i=0;i<(unsigned)list.size();++i)
        {
            if (list.at(i).id == id)
                return &list[i];
        }

        return NULL;
    }

    Route *GetByI(unsigned int i)
    {
        if ((unsigned)list.size() > i)
            return &list[i];
        else
            return NULL;
    }

    Route *GetBetween(unsigned int firstId, unsigned int secondId)
    {
        if (firstId > secondId)
        {
            unsigned int tmp = firstId;
            firstId = secondId;
            secondId = tmp;
        }

        for (unsigned int i=0;i<(unsigned)list.size();++i)
        {
            if (list.at(i).firstCity == firstId && list.at(i).secondCity == secondId)
                return &list[i];
        }

        return NULL;
    }

    unsigned int Size()
    {
        return (unsigned)list.size();
    }

    unsigned int CitiesNo()
    {
        QList < unsigned int > tmp;
        for (unsigned int i=0;i<(unsigned)list.size();++i)
        {
            Route route = list.at(i);
            bool foundFirst = false;
            bool foundSecond = false;

            for (unsigned int j=0;j<(unsigned)tmp.size();++j)
            {
                if (tmp.at(j) == route.firstCity)
                {
                    foundFirst = true;
                }

                if (tmp.at(j) == route.secondCity)
                {
                    foundSecond = true;
                }
            }

            if (!foundFirst)
                tmp.push_back(route.firstCity);

            if (!foundSecond)
                tmp.push_back(route.secondCity);
        }

        return tmp.size();
    }

    QVector < unsigned int > GetNeighbors(unsigned int city)
    {
        QVector < unsigned int > result;

        for (unsigned int i=0;i<(unsigned)list.size();++i)
        {
            if (list.at(i).firstCity == city)
            {
                result.push_back(list.at(i).secondCity);
                continue;
            }
            else if (list.at(i).secondCity == city)
            {
                result.push_back(list.at(i).firstCity);
            }
        }

        return result;
    }

    void NormalizeTrails(CitiesList *list)
    {
        double greatest = 0;
        for(unsigned int i=0;i<(unsigned)list->Size();++i)
        {
            if (list->GetByI(i)->trails > greatest)
                greatest = list->GetByI(i)->trails;
        }

        if(greatest != 0)
        {
            for(unsigned int i=0;i<(unsigned)list->Size();++i)
            {
                list->GetByI(i)->trails /= greatest;
            }
        }
    }

    void Deposite(CitiesList *copy)
    {
        NormalizeTrails(copy);
        for(unsigned int i=0;i<(unsigned)list.size();++i)
        {
            Route *act = &list[i];
            act->trails = copy->GetById(act->id)->trails;
        }
    }

    void Evaporate()
    {
        for(unsigned int i=0;i<(unsigned)list.size();++i)
        {
            Route *route = &list[i];
            double delta = route->trails - route->lastTrails;
            route->trails = (1-Controls::startingPheromone) * route->trails + fabs(delta);
            route->lastTrails = route->trails;
        }
    }

    void SelectBest()
    {
        /*unsigned int actual = Controls::startingCity;
        unsigned int hist = Controls::startingCity;
        for (unsigned int i=0;i<this->CitiesNo();++i)
        {
            QVector < unsigned int > neighbors = this->GetNeighbors(actual);
            double best = 0;
            unsigned int id = 0;
            Route *bestRt = NULL;
            for (unsigned int j=0;j<(unsigned)neighbors.size();++j)
            {
                Route *act = GetBetween(actual, neighbors.at(j));

                unsigned int excluded = 0;
                if (act->firstCity == actual)
                    excluded = act->secondCity;
                else
                    excluded = act->firstCity;

                if (act->trails > best && excluded != hist)
                {
                    best = act->trails;
                    bestRt = act;
                }
            }

            hist = actual;

            if (bestRt != NULL)
            {
                bestRt->visited = true;
                qDebug() << "\n**********\n" << "From city " << bestRt->firstCity << " to city " << bestRt->secondCity << " with trails " << bestRt->trails;
                if (bestRt->firstCity != actual)
                    actual = bestRt->secondCity;
                else
                    actual = bestRt->firstCity;
            }
        }*/
    }
};

class Ant
{
public:
    unsigned int actualCity;
    CitiesList list;
    QHash < unsigned int, bool > lookupTable;

    Ant(CitiesList list) : actualCity(1), list(list)
    {
        unsigned int citiesNo = list.CitiesNo();
        for (unsigned int i=0;i<citiesNo;++i)
        {
            bool visited = (i+1 == Controls::startingCity) ? true : false;
            lookupTable.insert(i+1, visited);
        }
    }

    ~Ant() {}

    unsigned int GetActualCity()
    {
        return actualCity;
    }

    void VisitCity(unsigned int city)
    {
        lookupTable[city] = true;
    }

    unsigned int Go()
    {
        actualCity = MakeDecision();
        VisitCity(actualCity);
        return actualCity;
    }

    QVector < unsigned int > GetUnvisited()
    {
        QVector < unsigned int > result;
        for (unsigned int i=0;i<(unsigned)lookupTable.size();++i)
        {
            if (lookupTable.value(i+1) == false)
                result.push_back(i+1);
        }

        return result;
    }

    unsigned int MakeDecision()
    {
        QVector < DecisionEntry > decisionTable;

        double sumOfAllNeighbors = 0;
        //double sumOfPossible = 0;
        QVector < unsigned int > cities = GetUnvisited();
        //QVector < unsigned int > cities = this->list.GetNeighbors(actualCity);

        switch(cities.size())
        {
        case 0:
            return Controls::startingCity;
            break;
        case 1:
            return cities.at(0);
            break;
        }

        for (unsigned int i=0;i<(unsigned)cities.size();++i)
        {
            Route *route = list.GetBetween(actualCity, cities.at(i));

            double pheromone = route->trails;
            double oneOverDistance = 1.0f/double(route->distance);
            double result = pow(pheromone, Controls::alfa) * pow(oneOverDistance, Controls::beta);

            DecisionEntry entry;
            entry.cityId = cities.at(i);
            entry.coefficient = result;
            entry.visited = route->visited;

            sumOfAllNeighbors += result;

            decisionTable.push_back(entry);
        }

        // zero probability
        if (sumOfAllNeighbors < Controls::eps)
            return actualCity;

        for (unsigned int i=0;i<(unsigned)decisionTable.size();++i)
        {
            decisionTable[i].coefficient /= sumOfAllNeighbors;
        }

        qSort(decisionTable.begin(), decisionTable.end(), Controls::DecisionTableGetLower);

        qDebug() << "\nActual city: " << actualCity;
        for (unsigned int i=0;i<(unsigned)decisionTable.size();++i)
        {
            double prob = decisionTable.at(i).coefficient;
            qDebug() << "To city " << decisionTable.at(i).cityId << ": " << prob;
        }

        double random = Controls::random();
        qDebug() << "Random: " << random;

        unsigned int result = actualCity;
        for (unsigned int i=0;i<(unsigned)decisionTable.size();++i)
        {
            if (random <= decisionTable.at(i).coefficient)
            {
                result = decisionTable.at(i).cityId;
                break;
            }
        }

        if (result == actualCity)
            result = decisionTable.at(decisionTable.size()-1).cityId;

        return result;
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Controls::InitRandom();

    CitiesList list;
    list.AddRoute(1,2,3);
    list.AddRoute(1,3,3);
    list.AddRoute(1,4,4);

    list.AddRoute(2,1,1);
    list.AddRoute(2,3,2);
    list.AddRoute(2,4,2);

    list.AddRoute(3,1,3);
    list.AddRoute(3,2,2);
    list.AddRoute(3,4,6);

    list.AddRoute(4,1,4);
    list.AddRoute(4,2,2);
    list.AddRoute(4,3,6);

    list.PrintRoutes();

    for (unsigned int j=0;j<Controls::numberOfIterations;++j)
    {
        qDebug() << "----- Iteration [" << j << "] -----";
        CitiesList copy = list;
        for (unsigned int i=0;i<Controls::numberOfAnts;++i)
        {
            qDebug() << "::::: Ant [" << i << "] :::::";
            Ant ant(list);
            Route *route = NULL;
            unsigned int history;
            bool changed;
            do
            {
                changed = true;
                history = ant.GetActualCity();

                if (history == ant.Go())
                {
                    qDebug() << "We are stuck. End of the way?\n";
                    changed = false;
                }
                else
                {
                    qDebug() << "We are going to city " << ant.GetActualCity() << ". Yay!";
                    route = copy.GetBetween(history, ant.GetActualCity());

                    if (route != NULL)
                    {
                        route->trails += 1.0f/route->distance;
                    }
                }
            }
            while (changed);

            /*qDebug() << "";
            list.PrintRoutes();*/
        }

        list.Deposite(&copy);
        list.Evaporate();
        qDebug() << "\n***** Results *****\n";
        list.PrintRoutes();
    }

    list.SelectBest();

    return a.exec();
}
