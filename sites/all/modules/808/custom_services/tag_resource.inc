<?php

/*--------------------operations part-----------------------*/
function tag_resource_retrieve($tid , $parameter){
    $query = db_select("taxonomy_term_data" , "term_data");
    $query->leftJoin("field_data_field_image" , " field_image" , "term_data.tid = field_image.entity_id and field_image.entity_type = 'taxonomy_term' ");
    $query->leftJoin("file_managed" , "image" , "image.fid = field_image.field_image_fid");
    $query->leftJoin("field_data_field_meta_description" , "meta_description" , "term_data.tid = meta_description.entity_id and meta_description.bundle = 'latin_tags' ");
    $query->fields("term_data" , array("tid" , "name" , "description"));
    $query->addField("image" , "uri" , "image");
    $query->addField("meta_description" , "field_meta_description_value" , "meta_description");
    $query->condition("term_data.tid" , $tid);
    $tag = $query->execute()->fetch();
    if(empty($tag)) return false;

    if(!empty($tag->image)) $tag->image = generate_file_address($tag->image);

    global $user;
    $bookmark = db_select("flagging")
        ->fields("flagging" , array("flagging_id"))
        ->condition("entity_id" , $tid)->condition("entity_type" , "taxonomy_term")->condition("uid" , $user->uid)
        ->execute()->fetch();
    $tag->user_bookmark = (!empty($bookmark))? true : false;

    return $tag;
}