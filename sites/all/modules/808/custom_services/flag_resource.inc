<?php

/*-----------------------------------target actions-------------------------------------*/
function flag_resource_create($data){
    global $user;

    if(strcmp($data["type"] , "node") == 0){
        $type = db_select("node")->fields("node" , array("type"))->condition("nid" , $data["entity_id"])->execute()->fetch();
        if(empty($type)) return false;
        $type = "node";
        $flag_type = 1;
    }
    elseif(strcmp($data["type"] , "taxonomy") == 0){
        $type = db_select("taxonomy_term_data")->fields("taxonomy_term_data" , array("tid"))->condition("tid" , $data["entity_id"])->execute()->fetch();
        if(empty($type)) return false;
        $type = "taxonomy_term";
        $flag_type = 2;
    }
    else return false;

    if(!$user->uid > 0) return services_error("User is not logged in!" , 18);

    if(strcmp($data['action'] , "flag") == 0 && bookmark_status($data["entity_id"] , $user->uid , $flag_type) == false){
        $fid = db_insert('flagging')
            ->fields(array(
                'fid' => $flag_type,
                'entity_type' => $type,
                'entity_id' => $data["entity_id"],
                'uid' => $user->uid,
                'sid' => 0,
                'timestamp' => REQUEST_TIME
            ))
            ->execute();

        if($fid > 0) return true;
        else return false;
    }
    elseif(strcmp($data['action'] , "unflag") == 0 && bookmark_status($data["entity_id"] , $user->uid , $flag_type) == true){
        $num_deleted = db_delete('flagging')
            ->condition('uid', $user->uid)
            ->condition('entity_id', $data["entity_id"])
            ->condition('fid', $flag_type)
            ->execute();

        if($num_deleted == 1) return true;
        else return false;
    }
    else return false;
}
function bookmark_status($entity_id , $uid , $flag_type){
    $query = db_select('flagging' , 'flagging')
        ->condition('fid' , $flag_type)
        ->condition('entity_id' , $entity_id)
        ->condition('uid' , $uid);
    $count = $query->countQuery()->execute()->fetchField();

    if($count > 0) return true;
    else return false;
}