package com.amoeba.springreader.repository;

import org.springframework.data.mongodb.repository.MongoRepository;

import com.amoeba.springreader.domain.Role;

public interface RoleRepository extends MongoRepository<Role, String> {
}
