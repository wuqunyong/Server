/*
 Copyright (c) 2007-2011 iMatix Corporation
 Copyright (c) 2007-2011 Other contributors as noted in the AUTHORS file

 This file is part of 0MQ.

 0MQ is free software; you can redistribute it and/or modify it under
 the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 0MQ is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>

#include "Command.h"
#include "../Db/ResultSet.h"

void FREEZE_NET::DeallocateCommand(Command* cmd)
{
	switch (cmd->type_)
	{
	case Command::RPC_MYSQL_NOTIFY:
		{
			if (cmd->args_.RPC_MYSQL_NOTIFY.table_)
			{
				delete[] cmd->args_.RPC_MYSQL_NOTIFY.table_;
				cmd->args_.RPC_MYSQL_NOTIFY.table_ = NULL;
			}
			
			if (cmd->args_.RPC_MYSQL_NOTIFY.query_statement_)
			{
				delete[] cmd->args_.RPC_MYSQL_NOTIFY.query_statement_;
				cmd->args_.RPC_MYSQL_NOTIFY.query_statement_ = NULL;
			}
			
			break;
		}
	}
}
