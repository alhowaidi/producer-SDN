#include "producer.hpp"
#include "fstream"
#include <ostream>


namespace ndn {
namespace chunks {


Producer::Producer(const Name& prefix,
                   Face& face,
                   KeyChain& keyChain,
                   const security::SigningInfo& signingInfo,
                   time::milliseconds freshnessPeriod,
                   size_t maxSegmentSize,
		   std::string&is,
                   bool isVerbose,
                   bool needToPrintVersion
                   )
  : m_face(face)
  , m_keyChain(keyChain)
  , m_signingInfo(signingInfo)
  , m_freshnessPeriod(freshnessPeriod)
  , m_maxSegmentSize(maxSegmentSize)
  , m_isVerbose(isVerbose)
{
//  if (prefix.size() > 0 && prefix[-1].isVersion()) {
//    m_prefix = prefix.getPrefix(-1);
//    m_versionedPrefix = prefix;
//  }
//  else {
//prefix = is;
//std::string uri = "/ndn"+is;
// Name nuri(uri);
//prefix = nuri;
std::cout << prefix << std::endl;
std::cout << is << std::endl;
    m_prefix = prefix;
//is = m_prefix.toUri();
    m_versionedPrefix = Name(m_prefix);//.appendVersion();
//  }
//std::cerr << "m_prefix" << m_prefix << std::endl;
//std::cerr << "m_versionPrefix" << m_versionedPrefix << std::endl;
 populateStore(is);

  if (needToPrintVersion)
    std::cout << m_versionedPrefix[-1] << std::endl;

  m_face.setInterestFilter("/ndn/",
                           bind(&Producer::onInterest, this, _2),
                           RegisterPrefixSuccessCallback(),
                           bind(&Producer::onRegisterFailed, this, _1, _2));

//  if (m_isVerbose)
    std::cerr << "Data published with name: " << m_versionedPrefix << std::endl;
}

	void
Producer::run()
{
	m_face.processEvents();
}
std::string ss;
	void
Producer::onInterest(const Interest& interest)
{
	BOOST_ASSERT(m_store.size() > 0);

	if (m_isVerbose)
	{std::cerr << "Interest: " << interest << std::endl;
		std::cerr << "interest name: " << interest.getName() << std::endl;
	}
	const Name& name = interest.getName();
//	std::cerr << "Interest: " << interest << std::endl;
//	std::cerr << "interest name: " << name << std::endl;

	numInterest  = numInterest + 1;
//	std::cerr << " num interest: " << numInterest << std::endl;

	const auto s=name[1];

//	const std::string fileName = interest.getName()[1].toUri() +"/" +interest.getName()[2].toUri();
//	 std::cerr << "File name: " << ss << std::endl;
//	 std::cerr << "store size: " << m_store.size() << std::endl;

/*	if (m_store.size() == 0 || ss.compare(fileName)) { // first time call
		//std::ifstream& is;
		ss = fileName;
		// m_store.clear();
		std::cerr << "File name: " << "/cvmfs/cms.cern.ch/data/" << ss << std::endl;
		// m_prefix = name;
		// m_versionedPrefix = Name(m_prefix);//.appendVersion(i);
		//populateStore(ss);

	}else{
		//std::cout << "same name cont.." << std::endl;
	}
*/
	if(m_store.size() !=0)
	{
		shared_ptr<Data> data;

		bool lastSegment = false;

		// is this a discovery Interest or a sequence retrieval?
		if (name.size() == m_versionedPrefix.size() + 1 && m_versionedPrefix.isPrefixOf(name) &&
				name[-1].isSegment()) {
			const auto segmentNo = static_cast<size_t>(interest.getName()[-1].toSegment());
			// specific segment retrieval

			//    std::cerr << "in if"  << std::endl;
			if (segmentNo < m_store.size()) {
				data = m_store[segmentNo];
				//    std::cerr << "sno: " << segmentNo << std::endl;
				//    std::cerr << "*data: " << *data << std::endl;

				if(segmentNo == m_store.size() -1 )
					lastSegment = true;
			}

		}
		else if (interest.matchesData(*m_store[0])) {
			// Interest has version and is looking for the first segment or has no version
			data = m_store[0];
//			std::cerr << "in else if"  << std::endl;
			if(m_store.size() == 1)
				lastSegment = true;
			// std::cerr << data << std::endl;
			//std::cerr << "mstore[0]: "<< *m_store[0] << std::endl;
		}
		//  else{
		//    std::cerr << "in else!!" << std::endl;
		//     std::cerr << "*mstore[0]: "<< *m_store[0] << std::endl;
		//     std::cerr << "mstore[0]: "<< m_store[0] << std::endl;
		//  }

		if (data != nullptr) {
			if (m_isVerbose)
				std::cerr << "Data: " << *data << std::endl;

			if(lastSegment == true)
			{
				//	std::cerr << "clear store .. " << std::endl;
				//  	m_store.clear();
				//std::cerr << " num interest: " << numInterest << std::endl;

			}
//			std::cout << "sending the data back " << std::endl;
			m_face.put(*data);
		}
		else{
			std::cerr << " data is null .. " << std::endl;
		}
	}// if (m_store.size()!=0)
	//else
//	std::cerr << "empty store " << std::endl;
}

	void
Producer::populateStore(std::string fileNameO)
{
	//  numInterest  = numInterest + 1; 
	// std::cerr << " num interest: " << numInterest << std::endl;
	BOOST_ASSERT(m_store.size() == 0);

	std::string fileName = "/cvmfs/cms.cern.ch/data/" + fileNameO;  
	if (m_isVerbose)
		std::cerr << "Loading input ..." << std::endl;

	std::fstream is(fileName);

	if(is)
	{
		size_t fileSize = 0;
		is.seekg(0,std::ios::end);
		fileSize = is.tellg();
		is.seekg(0,std::ios::beg);
		totalNumByte = totalNumByte + fileSize;
		std::cerr << "file size: " << fileSize << std::endl;
		std::cerr << "Total Bytes: " << totalNumByte  << std::endl;

	}
	std::vector<uint8_t> buffer(m_maxSegmentSize);

	while (is.good()) {
		is.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
		const auto nCharsRead = is.gcount();
		if (nCharsRead > 0) {
			auto data = make_shared<Data>(Name(m_versionedPrefix).appendSegment(m_store.size()));

			data->setFreshnessPeriod(m_freshnessPeriod);
			data->setContent(&buffer[0], nCharsRead);

			m_store.push_back(data);
		}
	}


	if (m_store.empty()) 
	{
		//  auto data = make_shared<Data>(Name(m_versionedPrefix).appendSegment(0));
		//  data->setFreshnessPeriod(m_freshnessPeriod);
		//  m_store.push_back(data);

		fileName = "/cvmfs/cms.cern.ch/data/" + fileNameO + "P";
		std::fstream is(fileName);

		if(is)
		{
			size_t fileSize = 0;
			is.seekg(0,std::ios::end);
			fileSize = is.tellg();
			is.seekg(0,std::ios::beg);
			totalNumByte = totalNumByte + fileSize;

			std::cerr << "file size P: " << fileSize << std::endl;
			std::cerr << "Total Byte: " << totalNumByte << std::endl;

		}
		std::vector<uint8_t> buffer(m_maxSegmentSize);
		while (is.good()) {
			is.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
			const auto nCharsRead = is.gcount();
			if (nCharsRead > 0) {
				auto data = make_shared<Data>(Name(m_versionedPrefix).appendSegment(m_store.size()));

				data->setFreshnessPeriod(m_freshnessPeriod);
				data->setContent(&buffer[0], nCharsRead);

				m_store.push_back(data);
			}
		}


		if(m_store.empty())
		{
			fileName = "/cvmfs/cms.cern.ch/data/" + fileNameO + "C";
			std::fstream is(fileName);

			if(is)
			{
				size_t fileSize = 0;
				is.seekg(0,std::ios::end);
				fileSize = is.tellg();
				is.seekg(0,std::ios::beg);
				totalNumByte = totalNumByte + fileSize;

				std::cerr << "file size C: " << fileSize << std::endl;
				std::cerr << "Total Byte: " << totalNumByte << std::endl;


			}
			std::vector<uint8_t> buffer(m_maxSegmentSize);
			while (is.good()) {
				is.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
				const auto nCharsRead = is.gcount();
				if (nCharsRead > 0) {
					auto data = make_shared<Data>(Name(m_versionedPrefix).appendSegment(m_store.size()));

					data->setFreshnessPeriod(m_freshnessPeriod);
					data->setContent(&buffer[0], nCharsRead);

					m_store.push_back(data);
				}
			}

		}

		if(m_store.empty())
		{ return; }

	}

	auto finalBlockId = name::Component::fromSegment(m_store.size() - 1);
	for (const auto& data : m_store) {
		data->setFinalBlockId(finalBlockId);
		m_keyChain.sign(*data, m_signingInfo);
	}

	if (m_isVerbose)
		std::cerr << "Created " << m_store.size() << " chunks for prefix " << m_prefix << std::endl;
}

	void
Producer::onRegisterFailed(const Name& prefix, const std::string& reason)
{
	std::cerr << "ERROR: Failed to register prefix '"
		<< prefix << "' (" << reason << ")" << std::endl;
	m_face.shutdown();
}

} // namespace chunks
} // namespace ndn
