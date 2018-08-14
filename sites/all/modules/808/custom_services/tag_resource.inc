<?php

/*--------------------relationships part-----------------------*/
function tag_resource_contents($parameter, $tid){
    $query = db_select("taxonomy_term_data" , "term_data");
    $query->leftJoin("field_data_field_image" , " field_image" , "term_data.tid = field_image.entity_id and field_image.entity_type = 'taxonomy_term' ");
    $query->leftJoin("file_managed" , "image" , "image.fid = field_image.entity_id");
    $query->fields("term_data" , array("tid" , "name" , "description"));
    $query->addField("image" , "uri" , "tag_image");
    $query->condition("term_data.tid" , $tid);
    $tag = $query->execute()->fetch();

    $query = db_select("taxonomy_index","taxonomy_index");
    $query->fields("taxonomy_index" , array("nid"));
    $query->condition("taxonomy_index.tid" , $tid);
    $nids = $query->execute()->fetchCol();

    $query = db_select("node" , "node");
    $query->leftJoin("field_data_field_image" , "field_image" , "node.nid = field_image.entity_id and field_image.entity_type = 'node' ");
    $query->leftJoin("file_managed" , "picture" , "picture.fid = field_image.field_image_fid");
    $query->join("field_data_field_latin_contents_types" , "types" , "node.nid = types.entity_id");
    $query->join("taxonomy_term_data" , "term_data" , "types.field_latin_contents_types_tid = term_data.tid");
    $query->fields("node" , array("nid" , "title" , "created" , "changed"));
    $query->addField("picture" , "uri" , "picture");
    $query->addField("term_data" , "name" , "type");
    $query->condition("node.nid" , $nids , "IN");
    $query->condition("node.status" , 1);
    $nodes = $query->execute()->fetchAll();


    $contents = array();
    foreach ($nodes as $node){
        $node->picture = image_style_url("400x225" , $node->picture);
        $node->created = format_date($node->created);
        $node->changed = format_date($node->changed);

        if(!isset($contents[$node->type]))
            $contents[$node->type] = array();

        $key = $node->type;
        unset($node->type);
        array_push($contents[$key] , $node);
    }

    return ["tag" => $tag , "contents" => $contents];
}