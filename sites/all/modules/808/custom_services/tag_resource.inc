<?php

/*--------------------operations part-----------------------*/
function tag_resource_index($parameter){
    $page = isset($parameter["page"]) && !empty($parameter["page"]) ? $parameter["page"] : 0;
    $limits = isset($parameter["limits"]) && !empty($parameter["limits"]) ? $parameter["limits"] : 30;
    $sort_base = isset($parameter["sort"]) && !empty($parameter["sort"]) ? $parameter["sort"] : "newest";

    $query = db_select("taxonomy_term_data" , "tag");
    $query->fields("tag" , array("tid" , "name"));
    $query->condition("tag.vid" , 33);

    $query->leftJoin("field_data_field_image" , " field_image" , "tag.tid = field_image.entity_id and field_image.entity_type = 'taxonomy_term' ");
    $query->leftJoin("file_managed" , "image" , "image.fid = field_image.field_image_fid");
    $query->addField("image" , "uri" , "image");

    /*filter the response*/
    if(isset($parameter["search"]) && strlen($parameter["search"]) > 0){
        $value = $parameter["search"];
        $or = db_or();
        $or->condition("tag.name" , '%' . $value . '%', 'LIKE');
        $or->condition("tag.description" , '%' . $value . '%', 'LIKE');
        $query->condition($or);
    }

    /*sort the response*/
    switch ($sort_base){
        case "newest" :
            $query->orderBy("tag.tid" , "DESC");
            break;

        case "first" :
            $query->orderBy("tag.tid" , "ASC");
            break;

        default :
            $query->orderBy("tag.tid" , "DESC");
            break;
    }

    /*calculating the counts of response*/
    $count = clone ($query);

    /*filter to limited data*/
    $query->range($page * $limits , $limits);

    /*filter to limited data*/
    $tags = $query->execute()->fetchAll();


    foreach ($tags as $tag){
        $tag->image = !empty($tag->image) ? image_style_url("300x200" , $tag->image) : null;
    }
    return ["tags" => $tags , "count" => (int) $count->countQuery()->execute()->fetchField() , "page" => $page];
}

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
    if(empty($tag)) return services_error("Not Found!" , 404);

    if(!empty($tag->image)) $tag->image = generate_file_address($tag->image);

    global $user;
    $tag->user_follow = is_flagged($user->uid , $tid , "taxonomy_term");

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