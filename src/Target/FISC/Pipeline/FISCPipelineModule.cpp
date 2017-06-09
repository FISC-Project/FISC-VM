#include <fvm/Pass.h>
#include <stdio.h>

class PipelineModule : public Pass {
public:
	PipelineModule() : Pass(PASS_RUNTIME, 2) {}

	enum PassRetcode init() {
		printf("- Initializing Pipeline\n");
		return PASS_RET_OK;
	}

	enum PassRetcode finit() {
		printf("- Terminating Pipeline\n");
		return PASS_RET_OK;
	}

	enum PassRetcode run() {
		printf("Pipeline is running!\n");
		return PASS_RET_OK;
	}

	enum PassRetcode watchdog() {
		return PASS_RET_OK;
	}
};
