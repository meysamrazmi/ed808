<?php

/*-----------------------------------operations----------------------------------------*/
function contents_resource_index($parameter){
    /*
     * this part is for filters
     * search and filters on type, category, topic, tag
     */
    $custom_contents = array();
    $filter = false;
    if(isset($parameter["search"])){
        $custom_contents = search_content_by_value($parameter["search"]);
        $filter = true;
    }
    if(isset($parameter["type"])) {
        $tids = explode("," , $parameter["type"]);
        $query = db_select("field_data_field_latin_contents_types" , "type");
        $query->fields("type" , array("entity_id"));
        $query->condition("type.field_latin_contents_types_tid" , $tids , "IN");
        $results = $query->execute()->fetchCol();
        $custom_contents = (!$filter)? $results : array_intersect($custom_contents , $results);
        $filter = true;
    }
    if(isset($parameter["category"])){
        $tids = explode("," , $parameter["category"]);
        $query = db_select("field_data_field_latin_contents_categories" , "category");
        $query->fields("category" , array("entity_id"));
        $query->condition("category.field_latin_contents_categories_tid" , $tids , "IN");
        $results = $query->execute()->fetchCol();
        $custom_contents = (!$filter)? $results : array_intersect($custom_contents , $results);
        $filter = true;
    }
    if(isset($parameter["topic"])){
        $tids = explode("," , $parameter["topic"]);
        $query = db_select("field_data_field_latin_contents_topics" , "topic");
        $query->fields("topic" , array("entity_id"));
        $query->condition("topic.field_latin_contents_topics_tid" , $tids , "IN");
        $results = $query->execute()->fetchCol();
        $custom_contents = (!$filter)? $results : array_intersect($custom_contents , $results);
        $filter = true;
    }
    if(isset($parameter["tag"])){
        $query = db_select("taxonomy_index" , "taxonomy_index");
        $query->fields("taxonomy_index" , array("nid"));
        $query->condition("taxonomy_index.tid" , $parameter["tag"]);
        $results = $query->execute()->fetchCol();
        $custom_contents = (!$filter)? $results : array_intersect($custom_contents , $results);
        $filter = true;
    }
    if($filter == true && empty($custom_contents)) return ["contents" => array() , "count" => 0];

    /*fetch content list data*/
    $query = db_select("node" , "node");
    $query->leftJoin("field_data_field_image" , "field_image" , "node.nid = field_image.entity_id and field_image.entity_type = 'node' and field_image.bundle = 'latin_contents' and field_image.delta = '0' ");
    $query->leftJoin("file_managed" , "file" , "file.fid = field_image.field_image_fid");
    $query->fields("node" , array("nid" , "title" , "uid" , "created" , "changed"));
    $query->addField("file" , "uri" , "picture");
    $query->condition("node.status" , 1);
    $query->condition("node.type" , "latin_contents");
    if($filter == true) $query->condition("node.nid" , $custom_contents, "IN");
    $query->orderBy("node.nid" , "DESC");
    $count = clone ($query);
    $query->range($parameter["page"]*30 , 30);
    $contents = $query->execute()->fetchAll();
    if(empty($contents)) return ["contents" => array() , "count" => 0];

    $nids = array();
    $contents_r = array();
    foreach ($contents as $content){
        $content->created = format_date($content->created, "teaser");
        $content->changed = format_date($content->changed, "teaser");
        $content->picture = (!empty($content->picture)) ? image_style_url("300x200" , $content->picture) : "";

        array_push($nids , $content->nid);
        $content->types = array();
        $content->categories = array();
        $content->topics = array();

        $contents_r[$content->nid] = $content;
    }

    /*add each content's type*/
    $query = db_select("field_data_field_latin_contents_types" , "types");
    $query->fields("types" , array("entity_id"));
    $query->join("taxonomy_term_data" , "term_data" , "term_data.tid = types.field_latin_contents_types_tid");
    $query->fields("term_data" , array("tid" , "name"));
    $query->condition("types.entity_id" , $nids , "IN");
    $results = $query->execute()->fetchAll();
    foreach ($results as $result) array_push($contents_r[$result->entity_id]->types , array("tid" => $result->tid , "name" => $result->name));

    /*add each content's category*/
    $query = db_select("field_data_field_latin_contents_categories" , "categories");
    $query->fields("categories" , array("entity_id"));
    $query->join("taxonomy_term_data" , "term_data" , "term_data.tid = categories.field_latin_contents_categories_tid");
    $query->fields("term_data" , array("tid" , "name"));
    $query->condition("categories.entity_id" , $nids , "IN");
    $results = $query->execute()->fetchAll();
    foreach ($results as $result) array_push($contents_r[$result->entity_id]->categories , array("tid" => $result->tid , "name" => $result->name));

    /*add each content's topic*/
    $query = db_select("field_data_field_latin_contents_topics" , "topics");
    $query->fields("topics" , array("entity_id"));
    $query->join("taxonomy_term_data" , "term_data" , "term_data.tid = topics.field_latin_contents_topics_tid");
    $query->fields("term_data" , array("tid" , "name"));
    $query->condition("topics.entity_id" , $nids , "IN");
    $results = $query->execute()->fetchAll();
    foreach ($results as $result) array_push($contents_r[$result->entity_id]->topics , array("tid" => $result->tid , "name" => $result->name));

    $contents = array_values($contents_r);

    return ["contents" => $contents , "count" => (int) $count->countQuery()->execute()->fetchField() , "page" => $parameter["page"]];
}
function contents_resource_retrieve($nid , $parameter){
    /*fetch content data*/
    $query = db_select("node" , "node");
    $query->leftJoin("field_data_body" , "body" , "body.entity_id = node.nid and body.entity_type = 'node' and body.bundle = 'latin_contents' ");
    $query->leftJoin("field_data_field_image" , "field_image" , "field_image.entity_id = node.nid and field_image.entity_type = 'node' and field_image.delta = '0' ");
    $query->leftJoin("file_managed" , "image" , "image.fid = field_image.field_image_fid");
    $query->leftJoin("field_data_field_link" , "field_link" , "field_link.entity_id = node.nid");
    $query->leftJoin("field_data_field_eventtime" , "eventtime" , "node.nid = eventtime.entity_id");
    $query->leftJoin("field_data_field_event_time_and_duration" , "time_and_duration" , "node.nid = time_and_duration.entity_id");
    $query->leftJoin("field_data_field_event_place" , "event_place" , "node.nid = event_place.entity_id");
    $query->leftJoin("field_data_field_company" , "company" , "node.nid = company.entity_id");
    $query->leftJoin("field_data_field_event_registration_link" , "registration_link" , "node.nid = registration_link.entity_id");
    $query->leftJoin("field_data_field_ifwebinar" , "ifwebinar" , "node.nid = ifwebinar.entity_id");
    $query->leftJoin("field_data_field_event_808_suggestion" , "808_suggestion" , "node.nid = 808_suggestion.entity_id");
    $query->leftJoin("field_data_field_meta_description" , "meta_description" , "node.nid = meta_description.entity_id and meta_description.bundle = 'latin_contents' ");
    $query->fields("node" , array("nid" , "uid" , "title" , "created" , "changed"));
    $query->fields("body" , array("body_value"));
    $query->addField("image" , "uri" , "image");
    $query->addField("field_link" , "field_link_url" , "video_link");
    $query->addField("eventtime" , "field_eventtime_value" , "event_time");
    $query->addField("time_and_duration" , "field_event_time_and_duration_value" , "time_and_duration");
    $query->addField("event_place" , "field_event_place_value" , "event_place");
    $query->addField("company" , "field_company_value" , "company");
    $query->addField("registration_link" , "field_event_registration_link_url" , "registration_link");
    $query->addField("ifwebinar" , "field_ifwebinar_value" , "ifwebinar");
    $query->addField("808_suggestion" , "field_event_808_suggestion_value" , "808_suggestion");
    $query->addField("meta_description" , "field_meta_description_value" , "meta_description");
    $query->condition("node.nid" , $nid);
    $query->condition("node.status" , 1);
    $content = $query->execute()->fetch();
    if(empty($content)) return services_error("This content is not found!" , 404);

    /*post process, changes time format and add other information*/
    $content->created = format_date($content->created);
    $content->changed = format_date($content->changed);
    if(!empty($content->image)) $content->image = generate_file_address($content->image);
    if(isset($content->event_time)){
        $time = explode(" " , $content->event_time);
        $date = explode("-" , $time[0]);
        $times = explode(":" , $time[1]);
        $timestamp = mktime($times[0], $times[1], $times[2], $date[1], $date[2], $date[0]);
        $content->event_time = date("F j, Y" , $timestamp);
    }
    $content->references =  array();
    $content->files =  array();
    $content->type =  array();
    $content->category =  array();
    $content->topic =  array();
    $content->tags =  array();

    /*add files*/
    $query = db_select("field_data_field_files" , "field_files");
    $query->join("file_managed" , "file" , "field_files.field_files_fid = file.fid");
    $query->addField("file" , "uri" , "file");
    $query->condition("field_files.entity_id" , $content->nid);
    $results = $query->execute()->fetchAll();
    foreach ($results as $result) array_push($content->files , $result->file);

    /*add references*/
    $query = db_select("field_data_field_latin_content_references" , "ref");
    $query->addField("ref" , "field_latin_content_references_url" , "references_url");
    $query->condition("ref.entity_id" , $content->nid);
    $results = $query->execute()->fetchAll();
    foreach ($results as $result) array_push($content->references , $result->references_url);

    /*add types*/
    $query = db_select("field_data_field_latin_contents_types" , "types");
    $query->join("taxonomy_term_data" , "taxonomy" , "types.field_latin_contents_types_tid = taxonomy.tid");
    $query->fields("taxonomy" , array("tid" , "name"));
    $query->condition("types.entity_id" , $content->nid);
    $results = $query->execute()->fetchAll();
    foreach ($results as $result) array_push($content->type , array("tid" => $result->tid , "name" => $result->name));

    /*add categories*/
    $query = db_select("field_data_field_latin_contents_categories" , "categories");
    $query->join("taxonomy_term_data" , "taxonomy" , "categories.field_latin_contents_categories_tid = taxonomy.tid");
    $query->fields("taxonomy" , array("tid" , "name"));
    $query->condition("categories.entity_id" , $content->nid);
    $results = $query->execute()->fetchAll();
    foreach ($results as $result) array_push($content->category , array("tid" => $result->tid , "name" => $result->name));

    /*add topics*/
    $query = db_select("field_data_field_latin_contents_topics" , "topics");
    $query->join("taxonomy_term_data" , "taxonomy" , "topics.field_latin_contents_topics_tid = taxonomy.tid");
    $query->fields("taxonomy" , array("tid" , "name"));
    $query->condition("topics.entity_id" , $content->nid);
    $results = $query->execute()->fetchAll();
    foreach ($results as $result) array_push($content->topic , array("tid" => $result->tid , "name" => $result->name));

    /*add tags*/
    $query = db_select("field_data_field_latin_tags" , "tags");
    $query->join("taxonomy_term_data" , "taxonomy" , "tags.field_latin_tags_tid = taxonomy.tid");
    $query->fields("taxonomy" , array("tid" , "name"));
    $query->condition("tags.entity_id" , $content->nid);
    $results = $query->execute()->fetchAll();
    foreach ($results as $result) array_push($content->tags , array("tid" => $result->tid , "name" => $result->name));

    /*about author*/
    $query = db_select("users" , "user");
    $query->leftJoin("field_data_field_full_name" , "name" , "user.uid = name.entity_id");
    $query->leftJoin("field_data_field_about_me" , "about_me" , "user.uid = about_me.entity_id");
    $query->leftJoin("file_managed" , "file_managed" , "user.picture = file_managed.fid");
    $query->fields("user" , array("uid"));
    $query->addField("name" , "field_full_name_value" , "full_name");
    $query->addField("about_me" , "field_about_me_value" , "about_me");
    $query->addField("file_managed" , "uri" , "picture");
    $query->condition("user.uid" , $content->uid);
    $author = $query->execute()->fetch();

    /*check user if user bookmark this content or not*/
    global $user;
    $bookmark = db_select("flagging")
            ->fields("flagging" , array("flagging_id"))
            ->condition("entity_id" , $nid)->condition("entity_type" , "node")->condition("uid" , $user->uid)
            ->execute()->fetch();
    $content->user_bookmark = (!empty($bookmark))? true : false;

    return ["content" => $content , "author" => $author];
}

/*
 * search value in to the titles and bodies and tags
 * return array of nids
 */
function search_content_by_value($value){
    $query = db_select("node", "node");
    $query->fields('node', array('nid'));
    $query->condition('node.title', '%' . $value . '%', 'LIKE');
    $nid1 = $query->execute()->fetchCol();

    $query = db_select("field_data_body", "body");
    $query->addField('body', 'entity_id', 'nid');
    $query->condition('body.body_value', '%' . $value . '%', 'LIKE');
    $query->condition('body.entity_type' , 'node');
    $nid2 = $query->execute()->fetchCol();

    $query = db_select("taxonomy_term_data" , "term_data");
    $query->join("taxonomy_index" , "taxonomy_index" , "taxonomy_index.tid = term_data.tid");
    $query->fields("taxonomy_index" , array("nid"));
    $query->condition("term_data.name" , '%' . $value . '%', 'LIKE');
    $nid3 = $query->execute()->fetchCol();

    $nids = array_merge($nid1, $nid2);
    $nids = array_merge($nids , $nid3);

    return $nids;
}
/*-------------------------------------------------------------------------------------*/
/*-------------------------------------relationships-----------------------------------*/
function contents_resource_filters($parameter  , $type){
    $type = array();
    $category = array();
    $topic = array();

    /*
     * find the taxonomies for filters
     */
    $query = db_select("taxonomy_term_data" , "term_data");
    $query->fields("term_data" , array("tid" , "vid" , "name"));
    $query->condition("term_data.vid" , array(30 , 34 , 35) ,"IN");
    $results = $query->execute()->fetchAll();

    /*categorize each filter*/
    foreach($results as $result){
        if($result->vid == 30) array_push($type , array("tid" => $result->tid , "name" => $result->name));
        if($result->vid == 34) array_push($category , array("tid" => $result->tid , "name" => $result->name));
        if($result->vid == 35) array_push($topic , array("tid" => $result->tid , "name" => $result->name));
    }

    return array(
        array("name" => "type", "children" => $type),
        array("name" => "category", "children" => $category),
        array("name" => "topic", "children" => $topic)
    );
}
function content_resource_event_list($parameter , $url){
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
function content_resource_comments($parameter , $nid){
    $query = db_select("comment" , "comment");
    $query->join("field_data_comment_body" , "body" , "body.entity_id = comment.cid");
    $query->join("users" , "user" , "user.uid = comment.uid");
    $query->leftJoin("field_data_field_full_name" , "full_name" , "full_name.entity_id = user.uid and full_name.entity_type = 'user' ");
    $query->leftJoin("file_managed" , "user_pic" , "user_pic.fid = user.picture");
    $query->fields("comment" , array("cid" , "pid" , "uid" , "created" , "changed"));
    $query->addField("body" , "comment_body_value" , "body");
    $query->addField("user" , "name" , "author_username");
    $query->addField("full_name" , "field_full_name_value" , "author_name");
    $query->addField("user_pic" , "uri" , "author_pic");
    $query->condition("comment.nid" , $nid);
    $query->condition("comment.pid" , 0);
    $query->orderBy("comment.cid" , "ASC");
    $query->range(isset($parameter["page"])? $parameter["page"]*30 : 0 , 30);

    $subquery = db_select("comment" , "comment");
    $subquery->join($query , "parent" , "parent.cid = comment.pid");
    $subquery->join("field_data_comment_body" , "body" , "body.entity_id = comment.cid");
    $subquery->join("users" , "user" , "user.uid = comment.uid");
    $subquery->leftJoin("field_data_field_full_name" , "full_name" , "full_name.entity_id = user.uid and full_name.entity_type = 'user' ");
    $subquery->leftJoin("file_managed" , "user_pic" , "user_pic.fid = user.picture");
    $subquery->fields("comment" , array("cid" , "pid" , "uid" , "created" , "changed"));
    $subquery->addField("body" , "comment_body_value" , "body");
    $subquery->addField("user" , "name" , "author_username");
    $subquery->addField("full_name" , "field_full_name_value" , "author_name");
    $subquery->addField("user_pic" , "uri" , "author_pic");
    $subquery->orderBy("comment.cid" , "ASC");


    $comments = $query->execute()->fetchAll();
    $children = $subquery->execute()->fetchAll();

    $associate_comments = array();
    foreach ($comments as $comment){
        $comment->replies = array();
        $comment->created = format_date($comment->created);
        $comment->changed = format_date($comment->changed);
        if(!empty($comment->author_pic)) $comment->author_pic = image_style_url("200x200", $comment->author_pic);
        $associate_comments[$comment->cid] = $comment;
    }
    foreach ($children as $child){
        if(isset($associate_comments[$child->pid])) {
            $child->created = format_date($child->created);
            $child->changed = format_date($child->changed);
            if(!empty($child->author_pic)) $child->author_pic = image_style_url("200x200", $child->author_pic);
            array_push($associate_comments[$child->pid]->replies, $child);
        }
    }
    $comments = array_values($associate_comments);

    return $comments;
}
/*-------------------------------------------------------------------------------------*/
