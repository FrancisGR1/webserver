#include "ResourceRegistry.hpp"

ResourceRegistry::ResourceRegistry(ConnectionPool& pool, EventManager& events)
	: connection_pool_(pool)
	, events_(events) {}

void ResourceRegistry::add(int fd, Connection& conn, uint32_t events)
{
	//@TODO
}

void ResourceRegistry::modify(int fd, Connection& conn, uint32_t events)
{
	//@TODO
}
