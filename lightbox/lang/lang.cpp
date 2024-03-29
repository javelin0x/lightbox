#include "lang.h"

#include "../cfg/cfg.h"

namespace messages_en {
	std::string intro_msg = "[1] load default DB (" + cfg::_path + ")\n[2] load from alternate path\n[3] create new DB\ninput: ";
}

namespace messages_es {
	std::string intro_msg = "[1] cargar DB default (" + cfg::_path + ")\n[2] cargar DB de otro archivo\n[3] crear nueva DB\nentrada: ";
}

namespace messages {
	std::string intro_msg = "";
}

namespace lang {
	void set_english()
	{
		messages::intro_msg = messages_en::intro_msg;
	}
}