myDir = File.dirname(File.expand_path(__FILE__));
require "#{myDir}/../build-options.rb";

module Rakish

Rakish::CppProject.new(
	:name 			=> "artd-lib-logger",
	:package 		=> "artd",
	:id 			=> "6A9F2C14-564B-4817-86D1-E4877BEBC950", 
	:dependsUpon 	=> [
	]
) do

	cppDefine('BUILDING_artd_lib_logger');
		
	addPublicIncludes('include/artd/*.h');

    addSourceFiles(
        "Logger.cpp",
    );

    setupCppConfig :targetType =>'DLL' do |cfg|
    end
end

end # module Rakish

