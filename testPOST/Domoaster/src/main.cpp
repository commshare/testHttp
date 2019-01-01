#include "main.h"

using namespace domoaster;

int main(int argc, char** argv)
{

	if (setuid(0)) {
		perror("setuid");
		return 1;
	}

	Domoaster domoaster (argc, argv) ;

	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = exit_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);

	DOMOASTER_DEBUG << "Program start";

  domoaster.Start() ;

  // TODO: (Denia) Temporaire
  while (1) {
    sleep(1000);
  }

	DOMOASTER_INFO << "program end"; // end execution .
	return 0 ;
}

void exit_handler(int s) {
	DOMOASTER_INFO << "Caught signal: " << s;
	scheduler_standard();
	exit(1);
}

void scheduler_realtime() {
	struct sched_param p;
	p.__sched_priority = sched_get_priority_max(SCHED_RR);
	if (sched_setscheduler(0, SCHED_RR, &p) == -1) {
		perror("Failed to switch to realtime scheduler.");
	}
}

void scheduler_standard() {
	struct sched_param p;
	p.__sched_priority = 0;
	if (sched_setscheduler(0, SCHED_OTHER, &p) == -1) {
		perror("Failed to switch to normal scheduler.");
	}
}