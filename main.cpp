//Harrison Hayes
//Paris
//Operating Systems
//April 9th, 2018

#include <iostream>
#include <pthread.h>
#include <queue>
#include <sstream>
#include <unistd.h>
#include <stdint.h>
#include <vector>
#include <signal.h>

static int MAXCARS = -1;
static int totalcars = 0;
static int carcounter = 0;
static int bcars = 0;
static int wcars = 0;
static std::string DIRECTION = "";
static pthread_mutex_t car_lock, direction_lock;
static pthread_cond_t wb_can = PTHREAD_COND_INITIALIZER;
static pthread_cond_t bb_can = PTHREAD_COND_INITIALIZER;
static pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
static bool DONE = false;

struct Arrival{
	int time, duration, carNum;
	std::string bound;
	Arrival();
	Arrival(int t, int d, std::string b);
	void print();
};

void *tunnel(void *arg);
void *carBB(void *arg);
void *carThread(void *arg);
void *carWB(void *arg);

int main() {

	std::string s;
	std::queue<Arrival*> arrivalQ;

	//Wihle loop to read the input from the text file.
	int carNo = 1;
	while(getline(std::cin, s)) {
		if(s.empty()) continue;
		int t, d;
		std::string b;
		Arrival *temp;
		std::stringstream ss;
		//std::cout << "Line:" << s << std::endl;
		if(MAXCARS == -1) {
			MAXCARS = stoi(s);
		} else  {
			int num = 0;
			for(int i = 0; i < s.size(); i++) {
				if(s[i] != ' ') {
					ss << s[i];
				} else {
					if(num ==0) {
						t = stoi(ss.str());
						ss.str("");
						num++;
					} else if(num == 1) {
						b = ss.str();
						ss.str("");
						num++;
					}
				}
			}
			d = stoi(ss.str());
			temp = new Arrival(t, d, b);
			temp->carNum = carNo;
			if(temp->bound == "WB") wcars++;
			if(temp->bound == "BB") bcars++;

			//std::cout << temp->carNum << std::endl;
			arrivalQ.push(temp);
			carNo++;
		}
	}//while
	//printf("Data loaded\n");
	//std::cout << MAXCARS << std::endl;

	//create thread to update tunnel status every 5 seconds
	//tunnel();
	//int carNo = 1;
	int finished = arrivalQ.size();
	pthread_t tunnelid;
	pthread_create(&tunnelid, NULL, tunnel, NULL);
	std::vector<pthread_t> cars;
	while (!arrivalQ.empty()) {
		Arrival* a = arrivalQ.front();
		arrivalQ.pop();
		sleep(a->time);
		pthread_t carid;
		cars.push_back(carid);
//		if(a->bound == "WB"){
			//std::cout << totalcars << std::endl;
			pthread_create(&carid, NULL, carThread, a);
			carcounter++;
//		} else if(a->bound == "BB"){
//			pthread_create(&carid, NULL, carBB, a);
//		}
	}
	while(carcounter!=0){}
	std::cout << bcars << " Cars going to Bear Valley arrived at the tunnel" << std::endl;
	std::cout << wcars << " Cars going to Whittier Valley arrived at the tunnel" << std::endl;
	pthread_kill(tunnelid, 9);
	for(int i = 0; i < cars.size(); i++) {
		pthread_join(cars[i], NULL);
	}
	//tunnel(&carNo);
	//for each car input
		//read everything
		//sleep for time delay
		//create child thread

	//wait till all cars are terminated
	//terminate tunnel thread



	return 1;
}

Arrival::Arrival(){}
Arrival::Arrival(int t, int d, std::string b) {
	this->time = t;
	this->duration = d;
	this->bound = b;
	//this->print();
}

void Arrival::print(){
	std::cout<<this->time << "|" << this->bound
		<< "|" << this->duration << std::endl;

}

void *tunnel(void *arg){
	printf("Tunnel Process Starting\n");
	while (true) {
		pthread_mutex_lock(&direction_lock);
		DIRECTION = "WB";
		printf("The tunnel is now open to Whittier-bound traffic.\n");
		pthread_cond_broadcast(&wb_can);
		pthread_mutex_unlock(&direction_lock);
		sleep(5);
		pthread_mutex_lock(&direction_lock);
		DIRECTION = 'N';
		printf("The tunnel is now closed to ALL traffic.\n");
		pthread_mutex_unlock(&direction_lock);
        sleep(5);
		pthread_mutex_lock(&direction_lock);
		DIRECTION = "BB";
		printf("The tunnel is now open to Bear Valley-bound traffic.\n");
		pthread_cond_broadcast(&wb_can);
		pthread_mutex_unlock(&direction_lock);
		sleep(5);
		pthread_mutex_lock(&direction_lock);
		DIRECTION = 'N';
		printf("The tunnel is now closed to ALL traffic.\n");
		pthread_mutex_unlock(&direction_lock);
        sleep(5);
	} // while
}

void *carThread(void *arg){
	Arrival* a = (Arrival*) arg;
	pthread_mutex_lock(&direction_lock);
	//printf("Car #%d going to %s arrives at the tunnel\n",
	//	a->carNum, (*a).bound);
	std::cout <<"Car " << a->carNum << " going to " << a->bound 
		<< " arrives at the tunnel."<< std::endl;
	//while(DIRECTION is wrong or totalcars >= MAXCARS) wait(...)
	//check traffic direction
	while(DIRECTION != a->bound || totalcars >= MAXCARS) {
	//if needed wait for broadcast from tunnel
		pthread_cond_wait(&wb_can, &direction_lock);
	}
	
	
	//////pthread_mutex_unlock(&direction_lock);
	//////pthread_mutex_lock(&car_lock);
	//check current number of cars in tunnel
	//while(totalcars >= MAXCARS) {
	//if needed, wait for signal from exiting car
	//	pthread_cond_wait(&not_full, &car_lock);
	//}
	//increment # of cars in tunnel
	totalcars++;
	
	//printf("Car #%d going to %s enters the tunnel\n",
	//	a->carNum, a->bound);
	std::cout <<"Car " << a->carNum << " going to " << a->bound 
		<< " enters the tunnel."<< std::endl;
	
	pthread_mutex_unlock(&direction_lock);

	//sleep for duration of crossing time
	sleep(a->duration);

	////////pthread_mutex_lock(&car_lock);

	//decrement # of cars in tunnel
	pthread_mutex_lock(&direction_lock);
	totalcars--;
	pthread_cond_signal(&wb_can);
	pthread_mutex_unlock(&direction_lock);
	//send signal  to waiting cars

	//printf("Car #%d going to %s exits the tunnel\n",
	//	a->carNum, a->bound);
	

	//update counter
	pthread_mutex_lock(&direction_lock);
	std::cout <<"Car " << a->carNum << " going to " << a->bound 
		<< " exits the tunnel."<< std::endl;
	carcounter--;
	pthread_mutex_unlock(&direction_lock);

	pthread_mutex_unlock(&car_lock);

	//terminate
	pthread_exit((void*) 0);
}
