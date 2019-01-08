<?php
/*--------------------------operations part--------------------------------*/
function post_resource_create($data){
    global $user;
    if($user->uid == 0) return services_error("You should login first!" , 401);

    $node= new stdClass();
    $node->title  = $data['title'];
    $node->type  = "post";
    node_object_prepare($node);
    $node->language = "en";
    $node->uid  = $user->uid;
    $node->status = 0;
    $node->comment = 2;
    $node->created = REQUEST_TIME;
    node_save($node);

    $node->body['und'][0]['value'] = $data['body'];
    $node->body['und'][0]['summary'] = '';
    $node->body['und'][0]['format'] = 'full_html';
    node_save($node);

    return $node;
}