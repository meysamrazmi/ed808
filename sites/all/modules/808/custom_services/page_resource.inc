<?php
/*-----------------index part--------------------*/
function page_resource_index($args){
    if(strcmp($args , "about_us") == 0){
        $body = db_select("field_data_body" , "body")
                ->fields("body" , array("body_value"))
                ->condition("entity_id" , 19760)
                ->condition("bundle" , "page")
                ->execute()->fetch()->body_value;

        $query = db_select("field_data_field_image" , "field_image");
        $query->join("file_managed" , "image" , "field_image.field_image_fid = image.fid");
        $query->fields("image" , array("uri"));
        $query->condition("field_image.entity_id" , 19760);
        $query->condition("field_image.bundle" , "page");
        $image = $query->execute()->fetch();

        return ["body" => $body , "image" => generate_file_address($image->uri)];
    }
    if(strcmp($args , "contact_us") == 0){
        $body = db_select("field_data_body" , "body")
                ->fields("body" , array("body_value"))
                ->condition("entity_id" , 19768)
                ->condition("bundle" , "webform")
                ->execute()->fetch();
        if(!$body) $body = null;
        else $body = $body->body_value;

        $query = db_select("field_data_field_image" , "field_image");
        $query->join("file_managed" , "image" , "field_image.field_image_fid = image.fid");
        $query->fields("image" , array("uri"));
        $query->condition("field_image.entity_id" , 19768);
        $query->condition("field_image.bundle" , "webform");
        $image = $query->execute()->fetch();

        $query = db_select("webform_component" , "components");
        $query->fields("components" , array("form_key"));
        $query->condition("components.nid" , 19768);
        $contact_fields = $query->execute()->fetchCol();


        return ["body" => $body , "image" => generate_file_address($image->uri) , "contact_form" => $contact_fields];
    }
    return $args;
}
/*-----------------------------------------------*/