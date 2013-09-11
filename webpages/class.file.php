<?php
class File
{
    private static function path($file)
	{
		if(substr($file,0,1) == "/" && !strstr($file, $_SERVER["DOCUMENT_ROOT"]))
		{
			$file = $_SERVER["DOCUMENT_ROOT"] . $file;
		}
		return $file;
	}
	
	/*
	public static function include($file)
	{
		
		
	}
	*/
	
	public static function exsist($file)
	{
		return file_exists(self::path($file));
	}
	
    public static function read($file, $mode=null){
        if($file == null) return false;
		if($mode == null) $mode = FileReadModes::auto;
		$file = self::path($file);
	
		if(substr($file, 0, 7) == "http://" || substr($file, 0, 8) == "https://"){
			return "http and https is not supportet";
			//return file_get_contents($file);
		}

		#er det en php fil?
		if(strstr($file, ".php") && substr($file,0,6) != "http://" && ($mode == "auto" || $mode == "exec")){
			return self::include2string($file);
		}
		else if(is_file($file) && ($mode == "auto" || $mode == "plain")){
			return implode('', file($file));
		}
		else{
			return "File not found ($file)";
		}
    }
    public static function write($content,$file,$mode=""){
        
    }
    
    public static function minify($input_file, $output_file)
    {
      if(!function_exists("code"))
      {
        throw new Exception("Datatype: type.code.php needed");
      }
      $code = code(self::read($input_file))->minify(array_pop(explode(".", $input_file)));
      if($code !== false)
      {
        return self::write($code, $output_file);
      }
      
      return false;
    }
    
    public static function rename($old, $new)
    {
      
    }
    
    public static function delete($file)
    {
      
    }
    
    private static function include2string($filename)
    {
      $tmp = explode("?", self::path($filename));
      if(count($tmp) > 1){
        $myget = explode("&", $tmp[1]);
        $k="";
        $v="";
        foreach($myget as $g){
          list($k,$v) = explode("=", $g);
          $_GET[$k] = $v;
        }
      }
      if (self::exsist($tmp[0])) {
          ob_start();
          include $tmp[0];
          $contents = ob_get_contents();
          ob_end_clean();
          return $contents;
      }
      return false;
    }
        	
}

final class FileReadModes {
	const plain = "plain";
	const execute = "exec";
	const auto = "auto";
}
?>
