#ifndef REQUESTPROCESSOR_HPP
# define REQUESTPROCESSOR_HPP

class RequestProcessor : public IRequestHandler
{
	public:
		//@TODO adicionar informação da ligação
		RequestProcessor(const HttpRequest& request, const ServiceConfig& service);
		void process();
		bool done() const;
		const NewHttpResponse& response() const;
		~RequestProcessor();

	private:
		enum State 
		{
			Validating = 0,
			Resolving,
			Dispatching
			Handling,
			Done
		};

		const HttpRequest& m_request;
		State m_state;
		HttpRequestContext m_context;
		IRequestHandler* m_handler;
};

#endif // REQUESTPROCESSOR_HPP
