@echo off

@echo Stop LoginServer
tskill LoginServer

@echo Stop GatewayServer
tskill GatewayServer

@echo Stop RoleServer
tskill RoleServer

@echo Stop FightServer
tskill FightServer


pause