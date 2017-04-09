#include "hci_command.h"

namespace hci
{
	void command::complete()
	{
		m_cb(*this);
	}
};
