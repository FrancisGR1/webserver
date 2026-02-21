# Configurações
- [x] Structs auxiliares
- [x] Símbolos
- [x] Processamento
- [x] Mudar códigos de erro de size_t para StatusCode::Code
- [ ] "/" path dever ser ilegal
- [ ] Melhor log de erros
- [ ] Múltiplas configurações para testes
- [ ] Limpar código morto
- [ ] Separar ConfigType.* em vários ficheiros?
- [ ] Mudar LocationConfig e ServiceConfig para classes, incluir exists() em LocationConfig
- [ ] mudar nomes: listing = autoindex, default_file = index, upload_dir = upload_storage, 
- [ ] estrutura do diretório - https://wiki.debian.org/Nginx/DirectoryStructure
- [ ] parser - dividir em Lexer → Parser → AST → Semantic Validator → Config Builder

# HTTP - Receber Pedido
- [x] Processamento
    - [x] verificar se header existe antes de inserir
    - [x] lidar com restos no buffer depois de S::Done ou S::Error
    - [x] processar chunked body
    - [x] ignorar trailing headers
    - [x] overload do feed() com chars
    - [x] clear()
    - [ ] "/" path dever ser ilegal
    - [ ] melhorar log erros
- [ ] Implementar testes em tabela
- [ ] Enum de métodos/string correspondente http como StatusCode e Token

# HTTP - Resposta
- [x] GET    
- [x] POST
- [x] DELETE
- [ ] Reimplementar arquitetura - criar classes mais concretas e dividir responsabilidades
- [ ] Reorganizar ficheiros
- [x] Reimplementar body da resposta de modo a ser assíncrono
- [ ] Pequenas funções para throw dos códigos mais utilizados nos erros
- [ ] Os handlers não precisam de lidar diretame com setters da HttpResponse, criar funções da HttpResponse para fazer isso. Por exemplo: response.make_autoindex_response(), response.make_upload_response()

# CGI
- [ ] lidar com programas infinitos/timeouts, crashes, aborts, etc. Construir uma sandbox
- [ ] fazer class Timer para timeouts
- [ ] converter raw string em respostas?

# Gestão Eventos
# Gestão Clientes

# Utils
## Logger
- [x] Print básico
- [ ] O logger deve ser instanciável

## Path
- [ ] criar classes derivadas: ServerPath e CgiPath; ou então separar informação (struct CgiInfo)

# Compilação
- [x] Fazer Makefile

# Outros
- [ ] Melhor diagrama
