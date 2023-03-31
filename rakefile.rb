myDir = File.dirname(File.expand_path(__FILE__));
require "#{myDir}/../build-options.rb";

module Rakish

Rakish::CppProject.new(
	:name 			=> "artd-lib-logger",
	:package 		=> "artd",
	:dependsUpon 	=> []
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

