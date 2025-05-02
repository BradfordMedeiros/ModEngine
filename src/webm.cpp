#include "./webm.h"

// https://github.com/BradfordMedeiros/ModEngine/blob/5a5b6670135f8d9c489ae6851e663c76b5c3edcb/src/scene/types/video.cpp


static constexpr int kWebmReaderError = 1;
static constexpr int kWebmReaderEof = 2;
class WebmReader : public webm::Reader
{
public:
    WebmReader(std::string filepath){
    	this -> m_position = 0;
    	this -> filepath = filepath;
        //SDL_SeekIO(m_io, 0, SDL_IO_SEEK_SET);
		this -> file = fopen(filepath.c_str(), "rb");
		modassert(file != NULL, "did not open file");
    }
    ~WebmReader(){
    	std::cout << "WebmReader out of scope" << std::endl;
    	fclose(this -> file);
    }

    void Seek(std::uint64_t position)
    {
       std::cout << "WebmReader:Seek" << std::endl;
       fseek(this -> file, position, SEEK_SET);
       m_position = position;
    }

    webm::Status Skip(std::uint64_t num_to_skip, std::uint64_t* num_actually_skipped)
    {
    	std::cout << "WebmReader:Skip" << std::endl;
        auto value = fseek(this -> file, num_to_skip, SEEK_CUR);
        modassert(value == 0, "seek returned non zero value");
        m_position += num_to_skip;
        *num_actually_skipped = num_to_skip;
        return webm::Status(webm::Status::kOkCompleted);
    }

    webm::Status Read(std::size_t num_to_read, std::uint8_t* buffer, std::uint64_t* num_actually_read)
    {
    	std::cout << "WebmReader:Read" << std::endl;

    	auto bytesRead = fread(buffer, sizeof(std::uint8_t), num_to_read, this -> file);
    	*num_actually_read = bytesRead;
        this -> m_position += bytesRead;
        if (bytesRead > 0 && bytesRead == num_to_read){
            return webm::Status(webm::Status::kOkCompleted);
        }else if (bytesRead > 0 && bytesRead < num_to_read){
            return webm::Status(webm::Status::kOkPartial);
        }else if (bytesRead == 0){
           return webm::Status(kWebmReaderEof);
        }
       	modassert(false, "didnt code this case");
        return webm::Status(webm::Status::kOkPartial);
    }

    std::uint64_t Position() const
    {
       std::cout << "WebmReader:Position" << std::endl;
       return m_position;
    }

   // SDL_IOStream *m_io;
   std::uint64_t m_position;
   std::string filepath;
   FILE* file;
};

class WebmCallback : public webm::Callback
{
public:
    WebmCallback(){}

    webm::Status OnInfo(const webm::ElementMetadata &metadata, const webm::Info &info) override {
       std::cout << "WebmCallback:OnInfo" << std::endl;

       auto timescale = info.timecode_scale.value();
       auto duration = info.duration.value();
       auto title = info.title.value();

       std::cout << "WebmCallback::OnInfo timescale is: " << timescale << std::endl;
       std::cout << "WebmCallback::OnInfo duration: " << duration << std::endl;
       std::cout << "WebmCallback::OnInfo title: " << title << std::endl;

       return webm::Status(webm::Status::kOkCompleted);
    }

    webm::Status OnClusterBegin(const webm::ElementMetadata &metadata,
                                const webm::Cluster &cluster, webm::Action *action) override {
      std::cout << "WebmCallback:OnClusterBegin" << std::endl;
      return webm::Status(webm::Status::kOkCompleted);
    }

    webm::Status OnSimpleBlockBegin(const webm::ElementMetadata &metadata,
                                    const webm::SimpleBlock &simple_block,
                                    webm::Action *action) override {
      std::cout << "WebmCallback:OnSimpleBlockBegin" << std::endl;
      return webm::Status(webm::Status::kOkCompleted);
    }

    webm::Status OnSimpleBlockEnd(const webm::ElementMetadata &metadata,
                                  const webm::SimpleBlock &simple_block) override {
      std::cout << "WebmCallback:OnSimpleBlockEnd" << std::endl;
      return webm::Status(webm::Status::kOkCompleted);
    }

    webm::Status OnBlockBegin(const webm::ElementMetadata &metadata,
                              const webm::Block &block, webm::Action *action) override {
      std::cout << "WebmCallback:OnBlockBegin" << std::endl;
      return webm::Status(webm::Status::kOkCompleted);
    }

    webm::Status OnFrame(const webm::FrameMetadata &metadata, webm::Reader *reader,
                         std::uint64_t *bytes_remaining) override {
      std::cout << "WebmCallback:OnFrame" << std::endl;
      return Skip(reader, bytes_remaining);
    }

    webm::Status OnTrackEntry(const webm::ElementMetadata &metadata,
                              const webm::TrackEntry &track_entry) override {
      std::cout << "WebmCallback:OnTrackEntry" << std::endl;
      return webm::Status(webm::Status::kOkCompleted);
    }

private:
};




void webmTest(){
	std::cout << "webtest" << std::endl;
	{
  
	}
	exit(1);
}