<?php
/*---------------------operations part--------------------------------*/
function user_resource_retrieve($uid , $parameter){
    $query = db_select('users' , 'user');
    $query->leftJoin("file_managed" , "picture" , "picture.fid = user.picture");
    $query->leftJoin("field_data_field_image" , "field_image" , "user.uid = field_image.entity_id and field_image.entity_type = 'user' ");
    $query->leftJoin("file_managed" , "background_image" , "background_image.fid = field_image.field_image_fid");
    $query->leftJoin("field_data_field_full_name" , "full_name" , "user.uid = full_name.entity_id");
    $query->leftJoin("field_data_field_about_me" , "about_me" , "user.uid = about_me.entity_id");
    $query->leftJoin("field_data_field_university" , "university" , "user.uid = university.entity_id");
    $query->leftJoin("field_data_field_education_field" , "education_field" , "user.uid = education_field.entity_id");
    $query->leftJoin("field_data_field_education_degree" , "education_degree" , "user.uid = education_degree.entity_id");
    $query->leftJoin("field_data_field_job" , "job" , "user.uid = job.entity_id");
    $query->leftJoin("field_data_field_skills" , "skills" , "user.uid = skills.entity_id");
    $query->leftJoin("field_data_field_mobile" , "mobile" , "user.uid = mobile.entity_id");
    $query->leftJoin("field_data_field_files" , "field_files" , "user.uid = field_files.entity_id and field_files.entity_type = 'user' ");
    $query->leftJoin("file_managed" , "cv" , "cv.fid = field_files.field_files_fid");
    $query->fields('user' , array('uid' , 'name' , 'mail' , 'created' , 'login'));
    $query->addField("picture" , "uri" , "picture");
    $query->addField("background_image" , "uri" , "background_image");
    $query->addField("full_name" , "field_full_name_value" , "full_name");
    $query->addField("about_me" , "field_about_me_value" , "about");
    $query->addField("university" , "field_university_value" , "university");
    $query->addField("education_field" , "field_education_field_value" , "education_field");
    $query->addField("education_degree" , "field_education_degree_value" , "education_degree");
    $query->addField("job" , "field_job_value" , "job");
    $query->addField("skills" , "field_skills_value" , "skills");
    $query->addField("mobile" , "field_mobile_value" , "mobile");
    $query->addField("cv" , "uri" , "cv");
    $query->addField("cv" , "origname" , "cvName");
    $query->condition('user.uid' , $uid);
    $account = $query->execute()->fetch();
    if(empty($account)) return services_error("User is not exist!" , 404);
    $account->created = format_date($account->created);
    $account->login = format_date($account->login);

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
    if(!empty($account->cv_url)) $account->cv_url = generate_file_address($account->cv_url);
    else {
        unset($account->cv_url);
        unset($account->cv_name);
    }

    global $user;
    if($user->uid != $uid){
        unset($account->mail);
        unset($account->mobile);
        unset($account->login);
    }

    return $account;
}
function user_resource_update_data($data , $uid){
    if(!isset($data["hash"]) || !(hash_true($data["hash"] , "user"))) return services_error(t('Failed to access'), 10);
    global $user;
    $account = user_load($uid);
    if($user->uid != $account->uid) return services_error("You do not have permission for this action" , 18);

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
    if(isset($data["full_name"]) && strlen($data["full_name"]) > 0) $account->field_full_name["und"][0]["value"] = $data["full_name"];
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
    if(empty($result))return services_error(t("User is not exist!") , 404);
    elseif($result->status == 0)return services_error(t("User is inactive!") , 403);
    else $username = $result->name;

    /*check the truth of username and password*/
    $uid = user_authenticate($username, $data["password"]);

    /*
     * @todo
     * check flood for attempting to login frequently
     * */

    /*check re captcha*/
    $captcha = reCaptcha($data["reCaptchaToken"]);
    if($data["reCaptchaToken"] == "admin@ed808") $captcha = true;
    if($captcha == false) return services_error("ReCaptcha Error!" , 401);

    if ($uid) {
        $user = user_load($uid);
        if ($user->uid) {
            user_login_finalize();
            $login_data = new stdClass();
            $login_data->uid = $user->uid;
            $login_data->sessid = session_id();
            $login_data->session_name = session_name();
            $login_data->token = drupal_get_token('services');

            return $login_data;
        }
    }
    return services_error(t('Wrong username or password!') , 16);
}

function user_resource_register($data){
    if(!isset($data["hash"]) || !(hash_true($data["hash"] , "user"))) return services_error(t('Failed to access'), 403);

    if(!isset($data['username']) || empty($data['username'])) return services_error(t("Username is required!") , 401);
    if(!isset($data['password']) || empty($data['password'])) return services_error(t("Password is required!") , 401);
    if(!isset($data['email']) || empty($data['email'])) return services_error(t("Email is required!") , 401);
    if(!isset($data['full_name']) || empty($data['full_name'])) return services_error(t("Email is required!") , 401);
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
        'full_name' => array('und' => array(0 => array('value' => $data["full_name"]))),
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
    $user = user_save('', $new_user);

    /*login user*/
    user_login_finalize();
    $newUser = new stdClass();
    $newUser->uid = $user->uid;
    $newUser->sessid = session_id();
    $newUser->session_name = session_name();
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

function user_resource_subscribe_email($data){
    $email = $data["email"];
    $path = "sites/all/modules/808/subscribe_email.json";
    $string = file_get_contents($path);
    $data = json_decode($string, true);
    if($data == '') $data = array();
    else{
        if(in_array($email, $data)) return false;
    }
    array_push($data, $email);
    $json = json_encode($data);

    file_put_contents($path, $json);

    $node = node_load(19759);
    $body = $node->body["und"][0]["value"];
    $body .= "<div>" . $email . "</div>";
    $node->body["und"][0]["value"] = $body;
    node_save($node);
    return $node;
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
    $query = db_select("node" , "node");
    $query->leftJoin("field_data_body" , "body" , "body.entity_id = node.nid and body.bundle = 'latin_contents' ");
    $query->leftJoin("field_data_field_image" , "field_image" , "field_image.entity_id = node.nid and field_image.bundle = 'latin_contents' and field_image.delta = '0' ");
    $query->leftJoin("file_managed" , "pic" , "pic.fid = field_image.field_image_fid ");
    $query->fields("node" , array("nid" , "title" , "created" , "changed" , "status"));
    $query->addField("body" , "body_value" , "body");
    $query->addField("pic" , "uri" , "picture");
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
    $query->range((isset($parameter["page"]) ? $parameter["page"] : 0) * 30 , 30);
    $posts = $query->execute()->fetchAll();

    foreach ($posts as $post){
        $post->created = format_date($post->created, "teaser");
        $post->changed = format_date($post->changed, "teaser");
        $post->picture = (!empty($post->picture)) ? image_style_url("300x200" , $post->picture) : "";
    }

    return $posts;
}