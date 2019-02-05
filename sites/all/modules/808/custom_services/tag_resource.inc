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
/*----------------------------------------------------------*/
/*-----------------relationships part-----------------------*/
function tag_resource_relationships_list($parameter , $url){
    /*
     * This part return the most used tags in all contents
     * with pic and description
     * */
    if(strcmp($url , "home") == 0){
        /*subquery is used to find the most usage of the a tag in contents*/
        $subquery = db_select("field_data_field_latin_tags" , "contents");
        $subquery->fields("contents" , array("field_latin_tags_tid"));
        $subquery->condition("contents.entity_type" , "node");
        $subquery->condition("contents.bundle" , "latin_contents");
        $subquery->addExpression("COUNT(contents.entity_id)" , "contentsCount");
        $subquery->havingCondition("contentsCount" , 3 , ">");
        $subquery->groupBy("contents.field_latin_tags_tid");

        /*query returns an array of tags with "tid and name" with 5 indexes*/
        $query = db_select("taxonomy_term_data" , "tag");
        $query->join("field_data_field_image" , " field_image" , "tag.tid = field_image.entity_id and field_image.entity_type = 'taxonomy_term' ");
        $query->join("file_managed" , "image" , "image.fid = field_image.field_image_fid");
        $query->join($subquery , "contents" , "contents.field_latin_tags_tid = tag.tid");
        $query->fields("tag", array("tid" , "name"));
        $query->condition("tag.vid" , 33);
        $query->condition("tag.description" , '','!=');
        $query->isNotNull('tag.description');
        $query->orderBy("contents.contentsCount" , "DESC")->orderBy("tag.tid" , "DESC");
        $query->range(0 , 30);
        $result = $query->execute()->fetchAll();
        return $result;
    }
}
/*----------------------------------------------------------*/