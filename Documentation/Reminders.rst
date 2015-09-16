=========
Reminders
=========

Purpose
=======

This page is intended to be a repository for random bits of information that need to be remembered, 
but don't necessarily have a good alternative home.

Please try to keep it organized.

Using Mantis
============

* In the server config, `completed-file-key` is used to specify the routing key for sending an alert about each file completed; this can be turned off by making this an empty string.


Developing Mantis
=================

* Block size = record size * # of channels
* Request handler function signature: bool handler_class::handler_func(const param_node& a_payload, , const param_node& a_sender_node, const std::string& a_mantis_routing_key, request_reply_package& a_pkg )
* The ``mantis_routing_key`` in the request handler function signature is the message routing key without the queue name.