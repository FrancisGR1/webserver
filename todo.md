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

# HTTP - Receber Pedido
- [x] Processamento
    - [ ] melhorar log erros
    - [x] verificar se header existe antes de inserir
    - [x] lidar com restos no buffer depois de S::Done ou S::Error
    - [x] processar chunked body
    - [x] ignorar trailing headers
    - [x] overload do feed() com chars
    - [x] clear()
    - [ ] "/" path dever ser ilegal
- [ ] Implementar testes em tabela

# HTTP - Resposta
- [x] GET    
- [x] POST
- [x] DELETE

# CGI
# Servidor
- [x] Pesquisar como colocar um ip no bind ao inves de INADDR_ANY 
- [ ] caso um service tenha um IP e outro service tenha o mesmo IP, o que fazer?
- [x] RESQUEST 
# Gestão Eventos
# Gestão Clientes

# Logger
- [x] Print básico

# Compilação
- [x] Fazer Makefile

# Outros
- [ ] Diagrama
