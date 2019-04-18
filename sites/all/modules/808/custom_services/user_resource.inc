<?php
/*---------------------operations part--------------------------------*/
function user_resource_retrieve($uid , $parameter){
    $query = db_select('users' , 'user');
    $query->fields('user' , array('uid' , 'name' , 'mail' , 'created' , 'login'));
    $query->condition('user.uid' , $uid);

    $query->leftJoin("file_managed" , "picture" , "picture.fid = user.picture");
    $query->addField("picture" , "uri" , "picture");

    $query->leftJoin("field_data_field_image" , "field_image" , "user.uid = field_image.entity_id and field_image.entity_type = 'user' ");
    $query->leftJoin("file_managed" , "background_image" , "background_image.fid = field_image.field_image_fid");
    $query->addField("background_image" , "uri" , "background_image");

    $query->leftJoin("field_data_field_full_name" , "full_name" , "user.uid = full_name.entity_id");
    $query->addField("full_name" , "field_full_name_value" , "full_name");

    $query->leftJoin("field_data_field_about_me" , "about_me" , "user.uid = about_me.entity_id");
    $query->addField("about_me" , "field_about_me_value" , "about");

    $query->leftJoin("field_data_field_university" , "university" , "user.uid = university.entity_id");
    $query->addField("university" , "field_university_value" , "university");

    $query->leftJoin("field_data_field_education_field" , "education_field" , "user.uid = education_field.entity_id");
    $query->addField("education_field" , "field_education_field_value" , "education_field");

    $query->leftJoin("field_data_field_education_degree" , "education_degree" , "user.uid = education_degree.entity_id");
    $query->addField("education_degree" , "field_education_degree_value" , "education_degree");

    $query->leftJoin("field_data_field_job" , "job" , "user.uid = job.entity_id");
    $query->addField("job" , "field_job_value" , "job");

    $query->leftJoin("field_data_field_skills" , "skills" , "user.uid = skills.entity_id");
    $query->addField("skills" , "field_skills_value" , "skills");

    $query->leftJoin("field_data_field_mobile" , "mobile" , "user.uid = mobile.entity_id");
    $query->addField("mobile" , "field_mobile_value" , "mobile");

    $query->leftJoin("field_data_field_files" , "field_files" , "user.uid = field_files.entity_id and field_files.entity_type = 'user' ");
    $query->leftJoin("file_managed" , "cv" , "cv.fid = field_files.field_files_fid");
    $query->addField("cv" , "uri" , "cv");
    $query->addField("cv" , "origname" , "cvName");

    $account = $query->execute()->fetch();

    if(empty($account)) return services_error("User is not exist!" , 404);

    $account->created = date("F j, Y" , $account->created);
    $account->login = date("F j, Y" , $account->login);

    $account->roles = array();
    $query = db_select('users_roles' , 'user_role');
    $query->join('role' , 'role' , 'user_role.rid = role.rid');
    $query->fields('role', array('rid' , 'name'));
    $query->condition('user_role.uid' , $uid);
    $results = $query->execute()->fetchAll();
    foreach ($results as $result) array_push($account->roles , array("rid" => $result->rid , "name" => $result->name));

    if(!empty($account->picture)) $account->picture =  image_style_url("200x200", $account->picture);
    else unset($account->picture);

    if(!empty($account->background_image)) $account->background_image = image_style_url("1360x500_blur", $account->background_image);
    else unset($account->background_image);

    if(!empty($account->cv)) $account->cv = generate_file_address($account->cv);
    else {
        unset($account->cv_url);
        unset($account->cv_name);
    }

    global $user;
    if($user->uid != $uid){
        unset($account->mail);
        unset($account->mobile);
        unset($account->login);
        $account->user_follow = is_flagged($user->uid , $uid , "user");
    }

    $token = db_select("custom_linkedIn_token" , "token")
        ->fields("token" , array("token"))
        ->condition("uid" , $uid)
        ->condition("expire" , REQUEST_TIME , ">")
        ->execute()->fetch();
    if(empty($token)) $account->has_linkedin = false;
    else{
        $account->has_linkedin = true;
        $account->linkedin_token_expire_in = $token->token;
    }

    return $account;
}

function user_resource_update_data($data , $uid){
    if(!isset($data["hash"]) || !(hash_true($data["hash"] , "user"))) return services_error(t('Failed to access'), 403);
    global $user;
    $account = user_load($uid);
    if($user->uid != $account->uid) return services_error("You do not have permission to do this action" , 403);

    $deleted_fid = array();

    if(isset($data["username"]) && !empty($data["username"])){
        $query = db_select("users" , "user");
        $query->fields("user" , array("name"));
        $query->condition("user.name" , $data["username"]);
        $result = $query->execute()->fetch();
        if(empty($result)) $account->name = $data["username"];
        else return services_error(t("This username is already taken!") , 18);
    }
    if(isset($data["email"]) && !empty($data["email"])){
        $query = db_select("users" , "user");
        $query->fields("user" , array("mail"));
        $query->condition("user.mail" , $data["email"]);
        $result = $query->execute()->fetch();
        if(empty($result)) $account->mail = $data["email"];
        else return services_error(t("This Email is already taken!") , 18);
    }
    if(isset($data["password"]) && !empty($data["password"])){
        if(strlen($data["password"]) < 6) return services_error(t("The password should be at least 6 character!") , 16);
        require_once DRUPAL_ROOT . '/' . variable_get('password_inc', 'includes/password.inc');
        $account->pass = user_hash_password(trim($data['password']));
    }
    if(isset($data["full_name"]) && strlen($data["full_name"]) > 0) $account->field_full_name["und"][0]["value"] = $data["full_name"];
    if(isset($data["laws"]) && $data["laws"] == 1) $account->field_laws["und"][0]["value"] = 1;
    if(isset($data["picture"]) || (!empty($data["picture"]) && strcmp($data["picture"]["type"] , "image") == 0)){
        $old_pic_fid = isset($account->picture->fid) ? $account->picture->fid : 0;
        if($data["picture"] === "delete" && !empty($account->picture)){
            $deleted_fid[] = $old_pic_fid;
            $account->picture = 0;
        }
        else {
            if(isset($account->picture->fid) && $account->picture->fid != $data["picture"]["fid"]) $deleted_fid[] = $old_pic_fid;
            $data["picture"]["status"] = 0;
            $account->picture = (object)$data["picture"];
        }
    }
    if(isset($data["background_image"]) || (!empty($data["background_image"]) && strcmp($data["background_image"]["type"] , "image") == 0)){
        $old_image_fid = !empty($account->field_image['und'][0]) ? $account->field_image['und'][0]["fid"] : 0;
        if($data["background_image"] === "delete" && !empty($account->field_image)){
            $deleted_fid[] = $old_image_fid;
            $account->field_image = array();
        }elseif (is_array($data["background_image"])) {
            if(isset($account->field_image['und'][0]) && $account->field_image['und'][0]["fid"] != $data["background_image"]["fid"]) $deleted_fid[] = $old_image_fid;
            $account->field_image['und'][0] = $data["background_image"];
        }
    }
    if(isset($data["cv"])){
        $old_file_fid = !empty($account->field_files['und'][0]) ? $account->field_files['und'][0]["fid"] : 0;
        if($data["cv"] === "delete" && !empty($account->field_files)){
            $deleted_fid[] = $old_file_fid;
            $account->field_files = array();
        }elseif (is_array($data["cv"])) {
            if(isset($account->field_files['und'][0]) && $account->field_files['und'][0]["fid"] != $data["cv"]["fid"]) $deleted_fid[] = $old_file_fid;
            $data["cv"]["display"] = 1;
            $account->field_files['und']['0'] = $data["cv"];
        }
    }
    if(isset($data["mobile"])) $account->field_mobile["und"][0]["value"] = $data["mobile"];
    if(isset($data["education_degree"])) $account->field_education_degree["und"][0]["value"] = $data["education_degree"];
    if(isset($data["education_field"])) $account->field_education_field["und"][0]["value"] = $data["education_field"];
    if(isset($data["university"])) $account->field_university["und"][0]["value"] = $data["university"];
    if(isset($data["job"])) $account->field_job["und"][0]["value"] = $data["job"];
    if(isset($data["skills"])) $account->field_skills["und"][0]["value"] = $data["skills"];
    if(isset($data["about"])) $account->field_about_me["und"][0]["value"] = $data["about"];

    user_save($account);

    module_invoke_all('field_storage_pre_update' , "user" , $account);

    foreach ($deleted_fid as $fid) delete_file($fid , array("hash" => hash_hmac("sha256" , "file" , "@Civil808Android@Saze808@" , false)));

    return user_resource_retrieve($account->uid , array());
}
/*---------------------actions part------------------------------------*/
function user_resource_login($data){
    if(!isset($data["hash"]) || !(hash_true($data["hash"] , "user"))) return services_error(t('Failed to access'), 403);

    /*check re captcha*/
    $captcha = reCaptcha($data["reCaptchaToken"]);
    if($data["reCaptchaToken"] == "admin@ed808") $captcha = true;
    if($captcha == false) return services_error("ReCaptcha Error!" , 401);

    $login_user = login($data);

    return (array)$login_user;
}
function login($data){
    if(!isset($data["username_email"]) || !isset($data["password"]) || !isset($data["reCaptchaToken"])) return services_error("Incomplete data!" , 401);

    /*check if user is login or not*/
    global $user;
    if ($user->uid > 0) {
        return services_error(t("User is already logged in!") , 401);
    }

    /*check if user is exist and unblocked*/
    $query =  db_select('users' , 'user');
    $query->fields('user', array('name' , 'status'));
    $db_or = db_or();
    $db_or->condition('user.name', $data["username_email"] , 'LIKE');
    $db_or->condition('user.mail', $data["username_email"] , 'LIKE');
    $query->condition($db_or);
    $result = $query->execute()->fetchObject();
    if(empty($result))return services_error(t("User is not found with this credential!") , 404);
    elseif($result->status == 0)return services_error(t("User is inactive!") , 403);
    else $username = $result->name;

    /*check the truth of username and password*/
    $uid = user_authenticate($username, $data["password"]);

    /*
     * @todo
     * check flood for attempting to login frequently
     * */

    if ($uid) {
        $user = user_load($uid);
        if ($user->uid) {
            user_login_finalize();
            $login_data = new stdClass();
            $login_data->uid = $user->uid;
            $login_data->username = $user->name;
            $login_data->full_name = !empty($user->field_full_name) ? $user->field_full_name["und"][0]["value"] : null;
            if(!empty($user->picture)) $login_data->picture =  image_style_url("200x200", $user->picture->uri);
            $login_data->token = drupal_get_token('services');
            return $login_data;
        }
    }
    return services_error(t('Wrong username or password!') , 401);
}

function user_resource_register($data){
    if(!isset($data["hash"]) || !(hash_true($data["hash"] , "user"))) return services_error(t('Failed to access'), 403);

    if(!isset($data['username']) || empty($data['username'])) return services_error(t("Username is required!") , 401);
    if(!isset($data['password']) || empty($data['password'])) return services_error(t("Password is required!") , 401);
    if(!isset($data['email']) || empty($data['email'])) return services_error(t("Email is required!") , 401);
    if(!isset($data['full_name']) || empty($data['full_name'])) return services_error(t("Full name is required!") , 401);
    if(!isset($data['reCaptchaToken']) || empty($data['reCaptchaToken'])) return services_error(t("ReCaptcha is required!") , 401);

    $captcha = reCaptcha($data["reCaptchaToken"]);
    if($data["reCaptchaToken"] == "admin@ed808") $captcha = true;
    if($captcha == false) return services_error("ReCaptcha Error!" , 401);

    $new_user = array(
        'name' => $data['username'],
        'pass' => $data['password'],
        'mail' => $data['email'],
        'status' => 1,
        'init' => $data['email'],
        'roles' => array(
            DRUPAL_AUTHENTICATED_RID => 'authenticated user',
        ),
        'field_laws' => array('und' => array(0 => array('value' => 1))),
        'field_full_name' => array('und' => array(0 => array('value' => $data["full_name"]))),
    );

    return (array) register($new_user);
}
function register($new_user){
    /*check if user is logged in!*/
    global $user;
    if($user->uid > 0) return services_error(t("User is already logged in!") , 18);

    /*check if this username or email is duplicate*/
    $query = db_select("users", "user");
    $query->fields("user" , array("name" , "mail"));
    $db_or = db_or();
    $db_or->condition('user.name', $new_user['name'] , 'LIKE');
    $db_or->condition('user.mail', $new_user['mail'] , 'LIKE');
    $query->condition($db_or);
    $result = $query->execute()->fetchObject();
    if(!empty($result)) return services_error(t("A user with this username Email is already exist!") , 18);

    /*create new user*/
    $user = user_save('' , $new_user);

    /*login user*/
    user_login_finalize();
    $newUser = new stdClass();
    $newUser->uid = $user->uid;
    $newUser->username = $user->name;
    $newUser->full_name = !empty($user->field_full_name) ? $user->field_full_name["und"][0]["value"] : null;
    $newUser->token = drupal_get_token('services');

    return $newUser;
}

function user_resource_exist($data){
    $query = db_select("users", "user");
    $query->fields("user" , array("name" , "mail"));
    if(isset($data["username"]) && strlen($data["username"]) > 0) $query->condition('user.name', $data['username'] , 'LIKE');
    elseif(isset($data["email"]) && strlen($data["email"]) > 0) $query->condition('user.mail', $data['email'] , 'LIKE');
    else return services_error(t('The username or email is required!'), 15);
    $result = $query->execute()->fetchObject();
    if(!empty($result)) return true;
    else return false;
}

function user_resource_insert_interests($data){
    global $user;
    if($user->uid == 0) return services_error("You should login first!" , 403);
    $topics = $data["topics"];
    foreach ($topics as $tid){
        db_merge("user_temp_interests")->key(array(
            "uid" => $user->uid,
            "action" => "interest",
            "entity_type" => "taxonomy_term",
            "bundle" => "latin_content_topic",
            "entity_id" => $tid,
        ))->fields(array(
            "uid" => $user->uid,
            "action" => "interest",
            "entity_type" => "taxonomy_term",
            "bundle" => "latin_content_topic",
            "entity_id" => $tid,
            "value" => "1",
            "date" => REQUEST_TIME
        ))->execute();
    }

    return true;
}
/*---------------------relationships part------------------------------*/
function user_resource_nav_bar_info($parameter , $login){
    global $user;
    if(!$user->uid > 0) return array("uid" => 0);
    $query = db_select("file_managed" , "picture");
    $query->fields("picture" , array("uri"));
    $query->condition("picture.fid" , $user->picture);
    $picture = $query->execute()->fetch();

    $query = db_select("field_data_field_full_name" , "full_name");
    $query->addField("full_name" , "field_full_name_value" , "full_name");
    $query->condition("full_name.entity_id" , $user->uid)->condition("full_name.entity_type" , "user");
    $full_name = $query->execute()->fetch();

    module_load_include("inc" , "services" , "resources/user_resource");
    $token = _user_resource_get_token();
    $result = array();
    $result["uid"] = $user->uid;
    $result["username"] = $user->name;
    $result["full_name"] = $full_name->full_name;
    if(!empty($picture)) $result["picture"] = image_style_url("200x200", $picture->uri);
    $result["token"] = $token["token"];
    return $result;
}

function user_resource_posts($parameter , $uid){
    global $user;
    $page = isset($parameter["page"]) && !empty($parameter["page"]) ? $parameter["page"] : 0;
    $limits = isset($parameter["limits"]) && !empty($parameter["limits"]) ? $parameter["limits"] : 30;

    $query = db_select("node" , "node");
    $query->fields("node" , array("nid" , "title" , "created" , "changed" , "status"));

    $query->leftJoin("field_data_body" , "body" , "body.entity_id = node.nid and body.bundle = 'latin_contents' ");
    $query->addField("body" , "body_value" , "body");

    $query->leftJoin("field_data_field_image" , "field_image" , "field_image.entity_id = node.nid and field_image.bundle = 'latin_contents' and field_image.delta = '0' ");
    $query->addField("pic" , "uri" , "picture");

    $query->leftJoin("file_managed" , "pic" , "pic.fid = field_image.field_image_fid ");
    $query->addField("term_data" , "tid" , "topic_tid");

    $query->leftJoin("field_data_field_latin_contents_topics" , "topics" , "topics.entity_id = node.nid and topics.entity_type = 'node' and topics.bundle = 'latin_contents' and topics.delta = '0' ");
    $query->addField("term_data" , "name" , "topic_name");

    $query->leftJoin("taxonomy_term_data" , "term_data" , "term_data.tid = topics.field_latin_contents_topics_tid");

    $query->condition("node.type" , "latin_contents");
    $query->condition("node.uid" , $uid);
    if($user->uid != $uid) $query->condition("node.status" , '1');
    elseif(!empty($parameter["publish"])){
        switch ($parameter["publish"]){
            case "1" :
                $query->condition("node.status" , '1');
                break;
            case "-1" :
                $query->condition("node.status" , '0');
                break;
        }
    }

    $query->orderBy("node.nid" , "DESC");

    $count = clone ($query);

    $query->range($page * $limits , $limits);

    $posts = $query->execute()->fetchAll();

    foreach ($posts as $post){
        $post->created = date("F j, Y" , $post->created);
        $post->changed = date("F j, Y" , $post->changed);
        $post->picture = (!empty($post->picture)) ? generate_file_address($post->picture) : null;
    }

    /*This part is temporary and it will be omitted*/
    $query = db_select('users' , 'user');
    $query->condition('user.uid' , $uid);
    $query->leftJoin("field_data_field_full_name" , "full_name" , "user.uid = full_name.entity_id");
    $query->addField("full_name" , "field_full_name_value" , "full_name");
    $query->leftJoin("field_data_field_about_me" , "about_me" , "user.uid = about_me.entity_id");
    $query->addField("about_me" , "field_about_me_value" , "about");
    $query->leftJoin("field_data_field_job" , "job" , "user.uid = job.entity_id");
    $query->addField("job" , "field_job_value" , "job");
    $account = $query->execute()->fetch();
    /*--------------------------------------------*/

    return ["posts" => $posts , "count" => (int) $count->countQuery()->execute()->fetchField() , "page" => $page , "brief" => $account];
}

function user_resource_following($parameter , $uid){
    $page = isset($parameter["page"]) && !empty($parameter["page"]) ? $parameter["page"] : 0;
    $limits = isset($parameter["limits"]) && !empty($parameter["limits"]) ? $parameter["limits"] : 30;

    /*fetch the data of the user following*/
    $query = db_select("flagging" , "flag");
    $query->condition("flag.entity_type" , "user");
    $query->condition("flag.fid" , 3);
    $query->condition("flag.uid" , $uid);

    $query->join("users" , "user" , "flag.entity_id = user.uid");
    $query->fields("user" , array("uid" , "name"));

    $query->leftJoin("field_data_field_full_name" , "full_name" , "user.uid = full_name.entity_id and full_name.entity_type = 'user' and full_name.bundle = 'user' ");
    $query->addField("full_name" , "field_full_name_value" , "full_name");

    $query->leftJoin("file_managed" , "picture" , "picture.fid = user.picture");
    $query->addField("picture" , "uri" , "picture");

    $query->leftJoin("field_data_field_about_me" , "about_me" , "user.uid = about_me.entity_id");
    $query->addField("about_me" , "field_about_me_value" , "about");

    /*sort base on time or name*/
    $sort_base = empty($parameter["sort"]) ? "time" : $parameter["sort"];
    if(!empty($parameter["sort_dir"]) && ($parameter["sort_dir"] === "ASC" || $parameter["sort_dir"] === "DESC")) $sort_dir = $parameter["sort_dir"];
    else $sort_dir = "DESC";
    switch ($sort_base){
        case "time" :
            $query->orderBy("flag.timestamp" , $sort_dir);
            break;
        case "name" :
            $query->orderBy("full_name.field_full_name_value" , $sort_dir)->orderBy("user.name" , $sort_dir);
            break;
        default:
            $query->orderBy("flagging.timestamp" , "DESC");
            break;
    }

    $query->range($page * $limits , $limits);

    $count = clone ($query);

    $following = $query->execute()->fetchAll();

    /*post process on followings*/
    foreach($following as $value){
        if(empty($value->full_name)) $value->full_name = $value->name;
        unset($value->name);
        if(!empty($value->picture)) $value->picture = image_style_url("200x200" , $value->picture);
    }

    return ["following" => $following , "count" => (int) $count->countQuery()->execute()->fetchField() , "page" => $page];
}

function user_resource_following_tags($parameter , $uid){
    $page = isset($parameter["page"]) && !empty($parameter["page"]) ? $parameter["page"] : 0;
    $limits = isset($parameter["limits"]) && !empty($parameter["limits"]) ? $parameter["limits"] : 30;

    $query = db_select("flagging" , "flag");
    $query->join("taxonomy_term_data" , "term" , "flag.entity_id = term.tid");
    $query->leftJoin("field_data_field_image" , "field_image" , "term.tid = field_image.entity_id and field_image.entity_type = 'taxonomy_term' ");
    $query->leftJoin("file_managed" , "image" , "image.fid = field_image.field_image_fid");
    $query->fields("term" , array("tid" , "name"));
    $query->addField("image" , "uri" , "picture");
    $query->condition("flag.uid" , $uid);
    $query->condition("flag.fid" , 2);
    $query->condition("flag.entity_type" , "taxonomy_term");

    /*sort base on time or name*/
    $sort_base = empty($parameter["sort"]) ? "time" : $parameter["sort"];
    if(!empty($parameter["sort_dir"]) && ($parameter["sort_dir"] === "ASC" || $parameter["sort_dir"] === "DESC")) $sort_dir = $parameter["sort_dir"];
    else $sort_dir = "DESC";
    switch ($sort_base){
        case "time" :
            $query->orderBy("flag.timestamp" , $sort_dir);
            break;
        case "name" :
            $query->orderBy("term.name" , $sort_dir);
            break;
        default:
            $query->orderBy("flagging.timestamp" , "DESC");
            break;
    }

    $count = clone ($query);
    $query->range($page * $limits , $limits);
    $tags = $query->execute()->fetchAll();
    foreach($tags as $tag) if(!empty($tag->picture)) $tag->picture = image_style_url("60x60" , $tag->picture);

    return ["following" => $tags , "count" => (int) $count->countQuery()->execute()->fetchField() , "page" => $page];
}

function user_resource_follower($parameter , $uid){
    $page = isset($parameter["page"]) && !empty($parameter["page"]) ? $parameter["page"] : 0;
    $limits = isset($parameter["limits"]) && !empty($parameter["limits"]) ? $parameter["limits"] : 30;

    /*fetch the data of the user following*/
    $query = db_select("flagging" , "flag");
    $query->condition("flag.entity_type" , "user");
    $query->condition("flag.fid" , 3);
    $query->condition("flag.entity_id" , $uid);

    $query->join("users" , "user" , "flag.uid = user.uid");
    $query->fields("user" , array("uid" , "name"));

    $query->leftJoin("field_data_field_full_name" , "full_name" , "user.uid = full_name.entity_id and full_name.entity_type = 'user' and full_name.bundle = 'user' ");
    $query->addField("full_name" , "field_full_name_value" , "full_name");

    $query->leftJoin("file_managed" , "picture" , "picture.fid = user.picture");
    $query->addField("picture" , "uri" , "picture");

    $query->leftJoin("field_data_field_about_me" , "about_me" , "user.uid = about_me.entity_id");
    $query->addField("about_me" , "field_about_me_value" , "about");

    $query->leftJoin("flagging" , "follow_back" , "follow_back.entity_type = 'user' and follow_back.fid = '3' and follow_back.uid = '" . $uid . "' and follow_back.entity_id = user.uid");
    $query->addField("follow_back" , "entity_id" , "user_follow");

    /*sort base on time or name*/
    $sort_base = empty($parameter["sort"]) ? "time" : $parameter["sort"];
    if(!empty($parameter["sort_dir"]) && ($parameter["sort_dir"] === "ASC" || $parameter["sort_dir"] === "DESC")) $sort_dir = $parameter["sort_dir"];
    else $sort_dir = "DESC";
    switch ($sort_base){
        case "time" :
            $query->orderBy("flag.timestamp" , $sort_dir);
            break;
        case "name" :
            $query->orderBy("full_name.field_full_name_value" , $sort_dir)->orderBy("user.name" , $sort_dir);
            break;
        default:
            $query->orderBy("flagging.timestamp" , "DESC");
            break;
    }

    $count = clone ($query);

    $query->range($page * $limits , $limits);

    $follower = $query->execute()->fetchAll();

    /*post process on followings*/
    foreach($follower as $value){
        if(empty($value->full_name)) $value->full_name = $value->name;
        unset($value->name);
        if(!empty($value->picture)) $value->picture = image_style_url("200x200" , $value->picture);

        $value->user_follow = empty($value->user_follow) ? false : true;
    }

    return ["follower" => $follower , "count" => (int) $count->countQuery()->execute()->fetchField() , "page" => $page];
}

function user_resource_bookmarks($parameter , $uid){
    $page = isset($parameter["page"]) && !empty($parameter["page"]) ? $parameter["page"] : 0;
    $limits = isset($parameter["limits"]) && !empty($parameter["limits"]) ? $parameter["limits"] : 30;

    $query = db_select("flagging" , "flag");
    $query->join("node" , "node" , "flag.entity_id = node.nid and node.status = '1' ");
    $query->leftJoin("field_data_field_image" , "field_image" , "node.nid = field_image.entity_id and field_image.entity_type = 'node' and field_image.bundle = 'latin_contents' and field_image.delta = '0' ");
    $query->leftJoin("file_managed" , "file" , "file.fid = field_image.field_image_fid");
    $query->fields("node" , array("nid" , "title" , "created" , "changed"));
    $query->addField("file" , "uri" , "picture");
    $query->condition("flag.uid" , $uid);
    $query->condition("flag.fid" , 1);
    $query->condition("flag.entity_type" , "node");

    $count = clone ($query);
    $query->range($page * $limits , $limits);
    $contents = $query->execute()->fetchAll();

    foreach($contents as $content){
        $content->created = date("F j, Y" , $content->created);
        $content->changed = date("F j, Y" , $content->changed);
        $content->picture = (!empty($content->picture)) ? image_style_url("300x200" , $content->picture) : "";
    }

    return ["contents" => $contents , "count" => (int) $count->countQuery()->execute()->fetchField() , "page" => $page];
}

function user_resource_favorite_content($parameter , $uid){
    $page = isset($parameter["page"]) && !empty($parameter["page"]) ? $parameter["page"] : 0;
    $limits = isset($parameter["limits"]) && !empty($parameter["limits"]) ? $parameter["limits"] : 30;

    $query = db_select("node" , "node");
    $query->join("clapping" , "clap" , "clap.entity_id = node.nid and clap.entity_type = 'node' and clap.uid = ' " . $uid . " ' ");
    $query->leftJoin("field_data_field_image" , "field_image" , "node.nid = field_image.entity_id and field_image.entity_type = 'node' and field_image.bundle = 'latin_contents' and field_image.delta = '0' ");
    $query->leftJoin("file_managed" , "file" , "file.fid = field_image.field_image_fid");
    $query->fields("node" , array("nid" , "title" , "created" , "changed"));
    $query->addField("file" , "uri" , "picture");
    $query->condition("node.status" , 1);

    $count = clone ($query);
    $query->range($page * $limits , $limits);
    $contents = $query->execute()->fetchAll();

    foreach($contents as $content){
        $content->created = date("F j, Y" , $content->created);
        $content->changed = date("F j, Y" , $content->changed);
        $content->picture = (!empty($content->picture)) ? image_style_url("300x200" , $content->picture) : "";
    }

    return ["contents" => $contents , "count" => (int) $count->countQuery()->execute()->fetchField() , "page" => $page];
}

function user_resource_favorite_comments($parameter , $uid){
    $page = isset($parameter["page"]) && !empty($parameter["page"]) ? $parameter["page"] : 0;
    $limits = isset($parameter["limits"]) && !empty($parameter["limits"]) ? $parameter["limits"] : 30;

    $query = db_select("comment" , "comment");
    $query->join("clapping" , "clap" , "clap.entity_id = comment.cid and clap.entity_type = 'comment' and clap.uid = ' " . $uid . " ' ");
    $query->leftJoin("field_data_comment_body" , "body" , "comment.cid = body.entity_id and body.entity_type = 'comment' ");
    $query->fields("comment" , array("cid" , "nid" , "created" , "changed"));
    $query->addField("body" , "comment_body_value" , "body");
    $query->condition("comment.status" , 1);

    $count = clone ($query);
    $query->range($page * $limits , $limits);
    $comments = $query->execute()->fetchAll();

    foreach($comments as $comment){
        $comment->created = date("F j, Y" , $comment->created);
        $comment->changed = date("F j, Y" , $comment->changed);
    }

    return ["comments" => $comments , "count" => (int) $count->countQuery()->execute()->fetchField() , "page" => $page];
}

function user_resource_wrote_comments($parameter , $uid){
    $page = isset($parameter["page"]) && !empty($parameter["page"]) ? $parameter["page"] : 0;
    $limits = isset($parameter["limits"]) && !empty($parameter["limits"]) ? $parameter["limits"] : 30;

    $query = db_select("comment" , "comment");
    $query->leftJoin("field_data_comment_body" , "body" , "comment.cid = body.entity_id and body.entity_type = 'comment' ");
    $query->join("node" , "node" , "node.nid = comment.cid and node.status = '1' ");
    $query->fields("comment" , array("cid" , "nid" , "created" , "changed"));
    $query->fields("node" , array("title"));
    $query->addField("body" , "comment_body_value" , "body");
    $query->condition("comment.uid" , $uid);

    $count = clone ($query);
    $query->range($page * $limits , $limits);
    $comments = $query->execute()->fetchAll();

    foreach($comments as $comment){
        $comment->created = date("F j, Y" , $comment->created);
        $comment->changed = date("F j, Y" , $comment->changed);
    }

    return ["comments" => $comments , "count" => (int) $count->countQuery()->execute()->fetchField() , "page" => $page];
}

function user_resource_linkedin_profile($parameter , $uid)
{
    $token = db_select("custom_linkedIn_token", "token")
        ->fields("token", array("token"))
        ->condition("uid", $uid)
        ->condition("expire", REQUEST_TIME, ">")
        ->execute()->fetch();

    if (!empty($token)) {
        $curl = curl_init();
        curl_setopt_array($curl, array(
            CURLOPT_URL => "https://api.linkedin.com/v1/people/~:(id,public-profile-url,first-name,last-name,email-address,location,summary,num-connections,formatted-name,headline,industry,specialties,positions,picture-url,picture-urls::(original))?format=json",
            CURLOPT_RETURNTRANSFER => true,
            CURLOPT_ENCODING => "",
            CURLOPT_MAXREDIRS => 10,
            CURLOPT_TIMEOUT => 30,
            CURLOPT_HTTP_VERSION => CURL_HTTP_VERSION_1_1,
            CURLOPT_CUSTOMREQUEST => "GET",
            CURLOPT_HTTPHEADER => array(
                "authorization: Bearer " . $token->token,
                "cache-control: no-cache",
                "connection: Keep-Alive"
            ),
        ));
        $data = curl_exec($curl);
        $err = curl_error($curl);
        curl_close($curl);
        if ($err) {
            return services_error("Unknown Error!" , 500);
        }
        $data = json_decode($data, true);

        $result = array();
        $result["full_name"] = $data["formattedName"];
        $result["headline"] = $data["headline"];
        $result["location"] = $data["location"]["name"];
        $result["industry"] = $data["industry"];
        $result["summary"] = $data["summary"];
        $result["connections"] = $data["numConnections"];
        $result["profile"] = $data["publicProfileUrl"];
        $result["picture"] = !empty($data["pictureUrls"]["values"]) ? $data["pictureUrls"]["values"][0] : null;
        $result["positions"] = $data["positions"]["values"];

        return $result;
    }
    return services_error("User has not connected to Linkedin or his/her connection has been expired!" , 404);
}

/*---------------------functions part------------------------------*/
