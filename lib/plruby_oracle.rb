module PLRubyOracle
  def self.init(name)
    @@encoding = case name
            when 'AL32UTF8'      then 'UTF-8'
            when 'JA16SJISTILDE' then 'Windows-31J'
            end
    if @@encoding.nil?
      require 'yaml'
      @@encoding = YAML::load_file(File.dirname(__FILE__) + '/plruby_oracle/encoding.yml')[name]
      if @@encoding.nil?
        raise "Ruby encoding name is not found in encoding.yml for #{name}."
      end
      if @@encoding.is_a? Array
        # Use the first available encoding in the array.
        @@encoding = enc.find do |e| Encoding.find(e) rescue false; end
      end
    end
  end
end
