/*******************************************************************************
 * Copyright 2013-2015 Aerospike, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/

#include <Python.h>
#include <structmember.h>
#include <stdbool.h>
#include <unistd.h>

#include <aerospike/aerospike.h>
#include <aerospike/as_config.h>
#include <aerospike/as_error.h>
#include <aerospike/as_policy.h>

#include "admin.h"
#include "client.h"
#include "policy.h"
#include "conversions.h"
#include "exceptions.h"

/*******************************************************************************
 * PYTHON TYPE METHODS
 ******************************************************************************/

static PyMethodDef AerospikeClient_Type_Methods[] = {

	// CONNECTION OPERATIONS

	{"connect",
		(PyCFunction) AerospikeClient_Connect, METH_VARARGS | METH_KEYWORDS,
		"Opens connection(s) to the cluster."},
	{"close",
		(PyCFunction) AerospikeClient_Close, METH_VARARGS | METH_KEYWORDS,
		"Close the connection(s) to the cluster."},
	{"is_connected",
		(PyCFunction) AerospikeClient_is_connected, METH_VARARGS | METH_KEYWORDS,
		"Checks current connection state."},
	{"shm_key",
		(PyCFunction) AerospikeClient_shm_key, METH_VARARGS | METH_KEYWORDS,
		"Get the shm key of the cluster"},

	// ADMIN OPERATIONS

	{"admin_create_user",
		(PyCFunction) AerospikeClient_Admin_Create_User, METH_VARARGS | METH_KEYWORDS,
		"Create a new user."},
	{"admin_drop_user",	(PyCFunction) AerospikeClient_Admin_Drop_User, METH_VARARGS | METH_KEYWORDS,
		"Drop a user."},
	{"admin_set_password",
		(PyCFunction) AerospikeClient_Admin_Set_Password,	METH_VARARGS | METH_KEYWORDS,
		"Set password"},
	{"admin_change_password",
		(PyCFunction) AerospikeClient_Admin_Change_Password, METH_VARARGS | METH_KEYWORDS,
		"Change password."},
	{"admin_grant_roles",
		(PyCFunction) AerospikeClient_Admin_Grant_Roles, METH_VARARGS | METH_KEYWORDS,
		"Grant Roles."},
	{"admin_revoke_roles",
		(PyCFunction) AerospikeClient_Admin_Revoke_Roles,	METH_VARARGS | METH_KEYWORDS,
		"Revoke roles"},
	{"admin_query_user",
		(PyCFunction) AerospikeClient_Admin_Query_User, METH_VARARGS | METH_KEYWORDS,
		"Query a user for roles."},
	{"admin_query_users",	(PyCFunction) AerospikeClient_Admin_Query_Users, METH_VARARGS | METH_KEYWORDS,
		"Query all users for roles."},
	{"admin_create_role",
		(PyCFunction) AerospikeClient_Admin_Create_Role, METH_VARARGS | METH_KEYWORDS,
		"Create a new role."},
	{"admin_drop_role",
		(PyCFunction) AerospikeClient_Admin_Drop_Role, METH_VARARGS | METH_KEYWORDS,
		"Drop a new role."},
	{"admin_grant_privileges",
		(PyCFunction) AerospikeClient_Admin_Grant_Privileges, METH_VARARGS | METH_KEYWORDS,
		"Grant privileges to a user defined role"},
	{"admin_revoke_privileges",
		(PyCFunction) AerospikeClient_Admin_Revoke_Privileges, METH_VARARGS | METH_KEYWORDS,
		"Revoke privileges from a user defined role"},
	{"admin_query_role",
		(PyCFunction) AerospikeClient_Admin_Query_Role, METH_VARARGS | METH_KEYWORDS,
		"Query a user defined role"},
	{"admin_query_roles",
		(PyCFunction) AerospikeClient_Admin_Query_Roles, METH_VARARGS | METH_KEYWORDS,
		"Querys all user defined roles"},

	// KVS OPERATIONS

	{"exists",
		(PyCFunction) AerospikeClient_Exists, METH_VARARGS | METH_KEYWORDS,
		"Check the existence of a record in the database."},
	{"get",
		(PyCFunction) AerospikeClient_Get, METH_VARARGS | METH_KEYWORDS,
		"Read a record from the database."},
	{"select",
		(PyCFunction) AerospikeClient_Select, METH_VARARGS | METH_KEYWORDS,
		"Project specific bins of a record from the database."},
	{"put",
		(PyCFunction) AerospikeClient_Put, METH_VARARGS | METH_KEYWORDS,
		"Write a record into the database."},
	{"remove",
		(PyCFunction) AerospikeClient_Remove, METH_VARARGS | METH_KEYWORDS,
		"Remove a record from the database."},
	{"apply",
		(PyCFunction) AerospikeClient_Apply, METH_VARARGS | METH_KEYWORDS,
		"Apply a UDF on a record in the database."},
	{"remove_bin",
		(PyCFunction) AerospikeClient_RemoveBin, METH_VARARGS | METH_KEYWORDS,
		"Remove a bin from the database."},
	{"append",
		(PyCFunction) AerospikeClient_Append, METH_VARARGS | METH_KEYWORDS,
		"Appends a string to the string value in a bin"},
	{"prepend",
		(PyCFunction) AerospikeClient_Prepend, METH_VARARGS | METH_KEYWORDS,
		"Prepend a record to the database"},
	{"touch",
		(PyCFunction) AerospikeClient_Touch, METH_VARARGS | METH_KEYWORDS,
		"Touch a record in the database"},
	{"increment",
		(PyCFunction) AerospikeClient_Increment, METH_VARARGS | METH_KEYWORDS,
		"Increments a numeric value in a bin"},
	{"operate",
		(PyCFunction) AerospikeClient_Operate, METH_VARARGS | METH_KEYWORDS,
		"Performs operate operation"},

	// Deprecated key-based API

	{"key",
		(PyCFunction) AerospikeClient_Key, METH_VARARGS | METH_KEYWORDS,
		"**[DEPRECATED]** Create a new Key object for performing key operations."},

	// QUERY OPERATIONS

	{"query",
		(PyCFunction) AerospikeClient_Query, METH_VARARGS | METH_KEYWORDS,
		"Create a new Query object for peforming queries."},
	{"query_apply",
		(PyCFunction) AerospikeClient_QueryApply, METH_VARARGS | METH_KEYWORDS,
		"Applies query object for performing queries."},
	{"job_info",
		(PyCFunction) AerospikeClient_JobInfo, METH_VARARGS | METH_KEYWORDS,
		"Gets Job Info"},

	// SCAN OPERATIONS

	{"scan",
		(PyCFunction) AerospikeClient_Scan, METH_VARARGS | METH_KEYWORDS,
		"Create a new Scan object for performing scans."},
	{"scan_apply",
		(PyCFunction) AerospikeClient_ScanApply, METH_VARARGS | METH_KEYWORDS,
		"Applies Scan object for performing scans."},

	{"scan_info",
		(PyCFunction) AerospikeClient_ScanInfo, METH_VARARGS | METH_KEYWORDS,
		"Gets Scan Info."},

	// INFO OPERATIONS
	{"info",
		(PyCFunction) AerospikeClient_Info, METH_VARARGS | METH_KEYWORDS,
		"Send an info request to the cluster."},
	{"info_node",
		(PyCFunction) AerospikeClient_InfoNode, METH_VARARGS | METH_KEYWORDS,
		"Send an info request to the cluster."},
	{"get_nodes",
		(PyCFunction) AerospikeClient_GetNodes, METH_VARARGS | METH_KEYWORDS,
		"Gets information about the nodes of the cluster."},
	{"has_geo",
		(PyCFunction)AerospikeClient_HasGeo, METH_VARARGS | METH_KEYWORDS,
		"Reflect if the server supports geospatial"},

	// UDF OPERATIONS

	{"udf_put",
		(PyCFunction)AerospikeClient_UDF_Put,	METH_VARARGS | METH_KEYWORDS,
		"Registers a UDF"},
	{"udf_remove",
		(PyCFunction)AerospikeClient_UDF_Remove, METH_VARARGS | METH_KEYWORDS,
		"De-registers a UDF"},
	{"udf_list",
		(PyCFunction)AerospikeClient_UDF_List, METH_VARARGS | METH_KEYWORDS,
		"Lists the UDFs"},
	{"udf_get",
		(PyCFunction)AerospikeClient_UDF_Get_UDF, METH_VARARGS | METH_KEYWORDS,
		"Get Registered UDFs"},

	// SECONDARY INDEX OPERATONS

	{"index_integer_create",
		(PyCFunction)AerospikeClient_Index_Integer_Create, METH_VARARGS | METH_KEYWORDS,
		"Creates a secondary integer index"},
	{"index_string_create",
		(PyCFunction)AerospikeClient_Index_String_Create,	METH_VARARGS | METH_KEYWORDS,
		"Creates a secondary string index"},
	{"index_remove",
		(PyCFunction)AerospikeClient_Index_Remove, METH_VARARGS | METH_KEYWORDS,
		"Remove a secondary index"},
	{"index_list_create",
		(PyCFunction)AerospikeClient_Index_List_Create, METH_VARARGS | METH_KEYWORDS,
		"Remove a secondary list index"},
	{"index_map_keys_create",
		(PyCFunction)AerospikeClient_Index_Map_Keys_Create, METH_VARARGS | METH_KEYWORDS,
		"Remove a secondary list index"},
	{"index_map_values_create",
		(PyCFunction)AerospikeClient_Index_Map_Values_Create, METH_VARARGS | METH_KEYWORDS,
		"Remove a secondary list index"},
	{"index_geo2dsphere_create",
		(PyCFunction)AerospikeClient_Index_2dsphere_Create,	METH_VARARGS | METH_KEYWORDS,
		"Creates a secondary geo2dsphere index"},

    // LSTACK OPERATIONS

	{"lstack",
		(PyCFunction) AerospikeClient_LStack, METH_VARARGS | METH_KEYWORDS,
		"LSTACK operations"},

    // LSET OPERATIONS

	{"lset",
		(PyCFunction) AerospikeClient_LSet, METH_VARARGS | METH_KEYWORDS,
		"LSET operations"},

    // LLIST OPERATIONS

	{"llist",
		(PyCFunction) AerospikeClient_LList, METH_VARARGS | METH_KEYWORDS,
		"LLIST operations"},

    // LMAP OPERATIONS

	{"lmap",
		(PyCFunction) AerospikeClient_LMap, METH_VARARGS | METH_KEYWORDS,
		"LMAP operations"},

	// BATCH OPERATIONS
	{"get_many",
		(PyCFunction)AerospikeClient_Get_Many, METH_VARARGS | METH_KEYWORDS,
		"Get many records at a time."},
	{"select_many",
		(PyCFunction)AerospikeClient_Select_Many, METH_VARARGS | METH_KEYWORDS,
		"Filter bins from many records at a time."},
	{"exists_many",
		(PyCFunction)AerospikeClient_Exists_Many, METH_VARARGS | METH_KEYWORDS,
		"Check existence of  many records at a time."},
	{"get_key_digest",
		(PyCFunction)AerospikeClient_Get_Key_Digest, METH_VARARGS | METH_KEYWORDS,
		"Get key digest"},

	{NULL}
};

/*******************************************************************************
 * PYTHON TYPE HOOKS
 ******************************************************************************/

static PyObject * AerospikeClient_Type_New(PyTypeObject * type, PyObject * args, PyObject * kwds)
{
	AerospikeClient * self = NULL;

	self = (AerospikeClient *) type->tp_alloc(type, 0);

	if ( self == NULL ) {
		return NULL;
	}

	return (PyObject *) self;
}

static int AerospikeClient_Type_Init(AerospikeClient * self, PyObject * args, PyObject * kwds)
{
	PyObject * py_config = NULL;

	static char * kwlist[] = {"config", NULL};

	if ( PyArg_ParseTupleAndKeywords(args, kwds, "O:client", kwlist, &py_config) == false ) {
		return -1;
	}

	if ( ! PyDict_Check(py_config) ) {
		return -1;
	}

	as_config config;
	as_config_init(&config);

	bool lua_system_path = FALSE;
	bool lua_user_path = FALSE;

	PyObject * py_lua = PyDict_GetItemString(py_config, "lua");
	if ( py_lua && PyDict_Check(py_lua) ) {

		PyObject * py_lua_system_path = PyDict_GetItemString(py_lua, "system_path");
		if ( py_lua_system_path && PyString_Check(py_lua_system_path) ) {
			lua_system_path = TRUE;
			memcpy(config.lua.system_path, PyString_AsString(py_lua_system_path), AS_CONFIG_PATH_MAX_LEN);
		}

		PyObject * py_lua_user_path = PyDict_GetItemString(py_lua, "user_path");
		if ( py_lua_user_path && PyString_Check(py_lua_user_path) ) {
			lua_user_path = TRUE;
			memcpy(config.lua.user_path, PyString_AsString(py_lua_user_path), AS_CONFIG_PATH_MAX_LEN);
		}

	}

	if ( ! lua_system_path ) {
		char system_path[AS_CONFIG_PATH_MAX_LEN] = {0};
		memcpy(system_path, "/usr/local/aerospike/lua", 24);
		system_path[24] = '\0';

		struct stat info;
		if (stat(system_path, &info) == 0 && (info.st_mode & S_IFDIR) && (access(system_path, R_OK)) == 0) {
			memcpy(config.lua.system_path, system_path, AS_CONFIG_PATH_MAX_LEN);
		}
		else {
			config.lua.system_path[0] = '\0';
		}
	}

	if ( ! lua_user_path ) {
		memcpy(config.lua.user_path, ".", AS_CONFIG_PATH_MAX_LEN);
	} else {
		struct stat info;
		if (stat(config.lua.user_path, &info ) != 0 || !(info.st_mode & S_IFDIR)) {
		    memcpy(config.lua.user_path, ".", AS_CONFIG_PATH_MAX_LEN);
		}
	}

	PyObject * py_hosts = PyDict_GetItemString(py_config, "hosts");
	if ( py_hosts && PyList_Check(py_hosts) ) {
		int size = (int) PyList_Size(py_hosts);
		for ( int i = 0; i < size && i < AS_CONFIG_HOSTS_SIZE; i++ ) {
			char *addr = NULL;
			uint16_t port = 3000;
			PyObject * py_host = PyList_GetItem(py_hosts, i);
			PyObject * py_addr, * py_port;

			if( PyTuple_Check(py_host) && PyTuple_Size(py_host) == 2) {

				py_addr = PyTuple_GetItem(py_host, 0);
				if (PyString_Check(py_addr)) {
					addr = strdup(PyString_AsString(py_addr));
				} else if (PyUnicode_Check(py_addr)) {
					PyObject * py_ustr = PyUnicode_AsUTF8String(py_addr);
					addr = strdup(PyString_AsString(py_ustr));
				}
				py_port = PyTuple_GetItem(py_host,1);
				if( PyInt_Check(py_port) || PyLong_Check(py_port) ) {
					port = (uint16_t) PyLong_AsLong(py_port);
				}
				else {
					port = 0;
				}
			}
			else if ( PyString_Check(py_host) ) {
				addr = strdup( strtok( PyString_AsString(py_host), ":" ) );
				addr = strtok(addr, ":");
				char *temp = strtok(NULL, ":");
				if(NULL != temp) {
					port = (uint16_t)atoi(temp);
				}
			}
			if(addr) {
				as_config_add_host(&config, addr, port);
			} else {
				free(addr);
				return -1;
			}
		}
	}

    PyObject * py_shm = PyDict_GetItemString(py_config, "shm");
    if (py_shm && PyDict_Check(py_shm) ) {

        config.use_shm = true;

        PyObject * py_shm_max_nodes = PyDict_GetItemString( py_shm, "shm_max_nodes" );
        if(py_shm_max_nodes && PyInt_Check(py_shm_max_nodes) ) {
            config.shm_max_nodes = PyInt_AsLong(py_shm_max_nodes);
        }

        PyObject * py_shm_max_namespaces = PyDict_GetItemString(py_shm, "shm_max_namespaces");
        if(py_shm_max_namespaces && PyInt_Check(py_shm_max_namespaces) ) {
            config.shm_max_namespaces = PyInt_AsLong(py_shm_max_namespaces);
        }

        PyObject* py_shm_takeover_threshold_sec = PyDict_GetItemString(py_shm, "shm_takeover_threshold_sec");
        if(py_shm_takeover_threshold_sec && PyInt_Check(py_shm_takeover_threshold_sec) ) {
            config.shm_takeover_threshold_sec = PyInt_AsLong( py_shm_takeover_threshold_sec);
        }

        PyObject* py_shm_cluster_key = PyDict_GetItemString(py_shm, "shm_key");
        if(py_shm_cluster_key && PyInt_Check(py_shm_cluster_key) ) {
            user_shm_key = true;
            config.shm_key = PyInt_AsLong(py_shm_cluster_key);
        }
    }

    self->is_client_put_serializer = false;
    self->user_serializer_call_info.callback = NULL;
    self->user_deserializer_call_info.callback = NULL;
    PyObject *py_serializer_option = PyDict_GetItemString(py_config, "serialization");
    if (py_serializer_option && PyTuple_Check(py_serializer_option)) {
        PyObject *py_serializer = PyTuple_GetItem(py_serializer_option, 0);
        if (py_serializer && py_serializer != Py_None) {
            if (!PyCallable_Check(py_serializer)) {
                return -1;
            }
            memset(&self->user_serializer_call_info, 0, sizeof(self->user_serializer_call_info));
            self->user_serializer_call_info.callback = py_serializer;
        }
        PyObject *py_deserializer = PyTuple_GetItem(py_serializer_option, 1);
        if (py_deserializer && py_deserializer != Py_None) {
            if (!PyCallable_Check(py_deserializer)) {
                return -1;
            }
            memset(&self->user_deserializer_call_info, 0, sizeof(self->user_deserializer_call_info));
            self->user_deserializer_call_info.callback = py_deserializer;
        }
    }

	as_policies_init(&config.policies);
    //Set default value of use_batch_direct
    config.policies.batch.use_batch_direct = false;

	PyObject * py_policies = PyDict_GetItemString(py_config, "policies");
	if ( py_policies && PyDict_Check(py_policies)) {
		//global defaults setting
		PyObject * py_key_policy = PyDict_GetItemString(py_policies, "key");
		if ( py_key_policy && PyInt_Check(py_key_policy) ) {
			config.policies.key = PyInt_AsLong(py_key_policy);
		}

		PyObject * py_timeout = PyDict_GetItemString(py_policies, "timeout");
		if ( py_timeout && PyInt_Check(py_timeout) ) {
			config.policies.timeout = PyInt_AsLong(py_timeout);
		}

		PyObject * py_retry = PyDict_GetItemString(py_policies, "retry");
		if ( py_retry && PyInt_Check(py_retry) ) {
			config.policies.retry = PyInt_AsLong(py_retry);
		}

		PyObject * py_exists = PyDict_GetItemString(py_policies, "exists");
		if ( py_exists && PyInt_Check(py_exists) ) {
			config.policies.exists = PyInt_AsLong(py_exists);
		}

		PyObject * py_replica = PyDict_GetItemString(py_policies, "replica");
		if ( py_replica && PyInt_Check(py_replica) ) {
			config.policies.replica = PyInt_AsLong(py_replica);
		}

		PyObject * py_consistency_level = PyDict_GetItemString(py_policies, "consistency_level");
		if ( py_consistency_level && PyInt_Check(py_consistency_level) ) {
			config.policies.consistency_level = PyInt_AsLong(py_consistency_level);
		}

		PyObject * py_commit_level = PyDict_GetItemString(py_policies, "commit_level");
		if ( py_commit_level && PyInt_Check(py_commit_level) ) {
			config.policies.commit_level = PyInt_AsLong(py_commit_level);
		}

		PyObject * py_max_threads = PyDict_GetItemString(py_policies, "max_threads");
        if ( py_max_threads && (PyInt_Check(py_max_threads) || PyLong_Check(py_max_threads))) {
            config.max_threads = PyInt_AsLong(py_max_threads);
        }

		PyObject * py_thread_pool_size = PyDict_GetItemString(py_policies, "thread_pool_size");
        if ( py_thread_pool_size && (PyInt_Check(py_thread_pool_size) || PyLong_Check(py_thread_pool_size))) {
            config.thread_pool_size = PyInt_AsLong(py_thread_pool_size);
        }

        //Setting for use_batch_direct
		PyObject * py_use_batch_direct = PyDict_GetItemString(py_policies, "use_batch_direct");
        if ( py_use_batch_direct && PyBool_Check(py_use_batch_direct)) {
            config.policies.batch.use_batch_direct = PyInt_AsLong(py_use_batch_direct);
        }

		/*
		 * Generation policy is removed from constructor.
		 */
	}

	//conn_timeout_ms
	PyObject * py_connect_timeout = PyDict_GetItemString(py_config, "connect_timeout");
	if ( py_connect_timeout && PyInt_Check(py_connect_timeout) ) {
		config.conn_timeout_ms = PyInt_AsLong(py_connect_timeout);
	}

	//strict_types check
	self->strict_types = true;
	PyObject * py_strict_types = PyDict_GetItemString(py_config, "strict_types");
	if ( py_strict_types && PyBool_Check(py_strict_types) ) {
		if (Py_False == py_strict_types) {
			self->strict_types = false;
 		}
	}

	self->as = aerospike_new(&config);

	return 0;
}

static void AerospikeClient_Type_Dealloc(PyObject * self)
{
    as_error err;
    as_error_init(&err);

    if (((AerospikeClient*)self)->as) {
        if (((AerospikeClient*)self)->as->config.hosts_size) {
            char * alias_to_search = return_search_string(((AerospikeClient*)self)->as);
            PyObject *py_persistent_item = NULL;

            py_persistent_item = PyDict_GetItemString(py_global_hosts, alias_to_search); 
            if (py_persistent_item) {
                close_aerospike_object(((AerospikeClient*)self)->as, &err, alias_to_search, py_persistent_item);
                ((AerospikeClient*)self)->as = NULL;
            }
            PyMem_Free(alias_to_search);
            alias_to_search = NULL;
        }
    }
	self->ob_type->tp_free((PyObject *) self);
}

/*******************************************************************************
 * PYTHON TYPE DESCRIPTOR
 ******************************************************************************/

static PyTypeObject AerospikeClient_Type = {
	PyObject_HEAD_INIT(NULL)

		.ob_size			= 0,
	.tp_name			= "aerospike.Client",
	.tp_basicsize		= sizeof(AerospikeClient),
	.tp_itemsize		= 0,
	.tp_dealloc			= (destructor) AerospikeClient_Type_Dealloc,
	.tp_print			= 0,
	.tp_getattr			= 0,
	.tp_setattr			= 0,
	.tp_compare			= 0,
	.tp_repr			= 0,
	.tp_as_number		= 0,
	.tp_as_sequence		= 0,
	.tp_as_mapping		= 0,
	.tp_hash			= 0,
	.tp_call			= 0,
	.tp_str				= 0,
	.tp_getattro		= 0,
	.tp_setattro		= 0,
	.tp_as_buffer		= 0,
	.tp_flags			= Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	.tp_doc				=
		"The Client class manages the connections and trasactions against\n"
		"an Aerospike cluster.\n",
	.tp_traverse		= 0,
	.tp_clear			= 0,
	.tp_richcompare		= 0,
	.tp_weaklistoffset	= 0,
	.tp_iter			= 0,
	.tp_iternext		= 0,
	.tp_methods			= AerospikeClient_Type_Methods,
	.tp_members			= 0,
	.tp_getset			= 0,
	.tp_base			= 0,
	.tp_dict			= 0,
	.tp_descr_get		= 0,
	.tp_descr_set		= 0,
	.tp_dictoffset		= 0,
	.tp_init			= (initproc) AerospikeClient_Type_Init,
	.tp_alloc			= 0,
	.tp_new				= AerospikeClient_Type_New
};

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

PyTypeObject * AerospikeClient_Ready()
{
	return PyType_Ready(&AerospikeClient_Type) == 0 ? &AerospikeClient_Type : NULL;
}

AerospikeClient * AerospikeClient_New(PyObject * parent, PyObject * args, PyObject * kwds)
{
	AerospikeClient * self = (AerospikeClient *) AerospikeClient_Type.tp_new(&AerospikeClient_Type, args, kwds);
	if ( AerospikeClient_Type.tp_init((PyObject *) self, args, kwds) == 0 ){
		// Initialize connection flag
		self->is_conn_16 = false;
		return self;
	}
	else {
		as_error err;
		as_error_init(&err);
		as_error_update(&err, AEROSPIKE_ERR_PARAM, "Parameters are incorrect");
		PyObject * py_err = NULL;
		error_to_pyobject(&err, &py_err);
		PyObject *exception_type = raise_exception(&err);
		PyErr_SetObject(exception_type, py_err);
		Py_DECREF(py_err);
		return NULL;
	}
}
