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
- [ ] mudar nomes: listing = autoindex, default_file = index, upload_dir = upload_storage, 
- [ ] estrutura do diretório - https://wiki.debian.org/Nginx/DirectoryStructure

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

# HTTP - Resposta
- [x] GET    
- [x] POST
- [x] DELETE

# CGI
- [ ] lidar com programas infinitos, crashes, aborts, etc. Construir uma sandbox
- [ ] converter raw string em respostas?

# Servidor
- [x] Pesquisar como colocar um ip no bind ao inves de INADDR_ANY 
- [ ] caso um service tenha um IP e outro service tenha o mesmo IP, o que fazer?
- [x] RESQUEST 
- [ ] CTRL+C e destrutores ??
# Gestão Eventos
# Gestão Clientes

# Logger
- [x] Print básico

# Compilação
- [x] Fazer Makefile

# Outros
- [ ] Melhor diagrama
