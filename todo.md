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
    - [ ] processa GET simples? (sem Transfer-Encoding e Content-Length)
- [ ] Implementar testes em tabela
- [ ] Enum de métodos/string correspondente http como StatusCode e Token

# HTTP - Resposta
- [x] GET    
- [x] POST
- [x] DELETE
- [x] Reimplementar arquitetura - criar classes mais concretas e dividir responsabilidades
- [x] Reorganizar ficheiros
- [x] Reimplementar body da resposta de modo a ser assíncrono
- [x] Pequenas funções para throw dos códigos mais utilizados nos erros

# CGI
- [ ] lidar com programas infinitos/timeouts, crashes, aborts, etc. Construir uma sandbox
- [x] converter raw string em respostas

# Servidor
- [x] Pesquisar como colocar um ip no bind ao inves de INADDR_ANY 
- [x] caso um service tenha um IP e outro service tenha o mesmo IP, o que fazer?
- [x] RESQUEST 
- [ ] CTRL+C e destrutores ??
- [ ] caso o client fique suspenso ou nao envie todo o request

# Gestão Eventos
- [x] passar EventManager para a ligacao -> contexto do processador

# Gestão Clientes
- [x] class Socket que guarde o contexto do servico do socket em si (fd)

# Ligacao
- [ ] implementar maquina de estados - receber - processar - enviar

# Utils
## Logger
- [x] Print básico
- [ ] O logger deve ser instanciável

## Path
- [ ] criar classes derivadas: ServerPath e CgiPath; ou então separar informação (struct CgiInfo)

# Compilação
- [x] Fazer Makefile

# Outros
- [ ] Melhor diagrama geral
- [ ] Inicializar todos os objetos: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es20-always-initialize-an-object

