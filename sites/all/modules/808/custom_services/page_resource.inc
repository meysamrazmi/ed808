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

        return ["body" => $body , "image" => $image->uri];
    }
    if(strcmp($args , "contact_us") == 0){
        $body = db_select("field_data_body" , "body")
                ->fields("body" , array("body_value"))
                ->condition("entity_id" , 19768)
                ->condition("bundle" , "page")
                ->execute()->fetch()->body_value;

        $query = db_select("field_data_field_image" , "field_image");
        $query->join("file_managed" , "image" , "field_image.field_image_fid = image.fid");
        $query->fields("image" , array("uri"));
        $query->condition("field_image.entity_id" , 19768);
        $query->condition("field_image.bundle" , "page");
        $image = $query->execute()->fetch();

        $query = db_select("webform_component" , "components");
        $query->fields("components" , array("form_key"));
        $query->condition("components.nid" , 19768);
        $contact_fields = $query->execute()->fetchCol();


        return ["body" => $body , "image" => $image->uri , "contact_form" => $contact_fields];
    }
    return $args;
}
/*-----------------------------------------------*/

/*----------------actions part-------------------*/
function page_resource_contact_form($data){
    if(!isset($data["name"])) return services_error("Name is required!" , 406);
    if(!isset($data["email"])) return services_error("Email is required!" , 406);
    if(!isset($data["description"])) return services_error("Description is required!" , 406);

    global $user;
    $node = node_load(19768);
    $new_data = array();
    $query = db_select("webform_component" , "components");
    $query->fields("components" , array("cid" , "form_key"));
    $query->condition("components.nid" , 19768);
    $components = $query->execute()->fetchAll();

    foreach ($components as $component) {
        if(isset($data[$component->form_key])) $new_data[$component->cid] = array($data[$component->form_key]);
        else $new_data[$component->cid] = array();
    }
    $submission = (object) array(
        'nid' => $node->nid,
        'uid' => $user->uid,
        'sid' => NULL,
        'submitted' => REQUEST_TIME,
        'completed' => 0,
        'modified' => REQUEST_TIME,
        'remote_addr' => webform_ip_address($node),
        'is_draft' => FALSE,
        'highest_valid_page' => 0,
        'preview' => False,
        'serial' => NULL,
        'data' => $new_data,
    );

    module_load_include('inc','webform','includes/webform.submissions');
    return webform_submission_insert($node , $submission);
}
/*-----------------------------------------------*/