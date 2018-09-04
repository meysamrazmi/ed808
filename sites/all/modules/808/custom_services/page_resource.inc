<?php

function page_resource_index($args){
    if(strcmp($args , "about_us") == 0){
        $body = db_select("field_data_body" , "body")
                ->fields("body" , array("body_value"))
                ->condition("entity_id" , 19760)
                ->condition("bundle" , "page")
                ->execute()->fetch()->body_value;

        return $body;
    }
    return $args;
}