<?php

/*---------------------------operations--------------------------*/
function event_resource_index($parameter){
    $query = db_select("node" , "node");
    $query->join("field_data_field_latin_contents_types" , "contents_types" , "contents_types.entity_id = node.nid and contents_types.field_latin_contents_types_tid = '4058' ");
    $query->join("field_data_field_eventtime" , "eventtime" , "eventtime.entity_id = node.nid");
    $query->leftJoin('field_data_field_company', 'company', 'company.entity_id = node.nid');
    $query->leftJoin('field_data_field_ifwebinar', 'ifwebinar', 'ifwebinar.entity_id = node.nid');
    $query->fields("node" , array("nid" , "title"));
    $query->addField("eventtime" , "field_eventtime_value" , "eventtime");
    $query->addField("company" , "field_company_value" , "company");
    $query->addField("ifwebinar" , "field_ifwebinar_value" , "webinar");
    $query->condition('node.status', 1);
    $query->condition('eventtime.field_eventtime_value', date("y-m-d h:m:s", time()) , '>=');
    $query->orderBy('eventtime.field_eventtime_value', 'ASC');
    $events = $query->execute()->fetchAll();
    return $events;
}

function event_resource_retrieve($nid , $parameter){
    $query = db_select("node" , "node");
    $query->leftJoin("field_data_body", "body", "body.entity_id = node.nid");
    $query->leftJoin("field_data_field_eventtime", "eventtime", "eventtime.entity_id = node.nid");
    $query->leftJoin("field_data_field_company", "company", "company.entity_id = node.nid");
    $query->leftJoin("field_data_field_ifwebinar", "ifwebinar", "ifwebinar.entity_id = node.nid");
    $query->leftJoin("field_data_field_image", "field_image", "field_image.entity_id = node.nid and field_image.entity_type = 'node' and field_image.bundle = 'latin_contents' ");
    $query->leftJoin("file_managed", "file", "file.fid = field_image.field_image_fid");
    $query->fields("node" , array("title"));
    $query->addField("body" , "body_value" , "body");
    $query->addField("eventtime" , "field_eventtime_value" , "eventtime");
    $query->addField("company" , "field_company_value" , "company");
    $query->addField("ifwebinar" , "field_ifwebinar_value" , "webinar");
    $query->fields("file" , array("uri"));
    $query->condition("node.nid" , $nid);
    $event = $query->execute()->fetch();
    
    return $event;
}