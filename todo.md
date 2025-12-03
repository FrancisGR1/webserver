# Configurações
- [x] Structs auxiliares
- [x] Símbolos
- [x] Processamento
- [ ] Melhor comunicação de erros
- [ ] Múltiplas configurações para testes

# HTTP - Receber Pedido
- [ ] Processamento
    - [ ] melhorar log erros
    - [ ] passar espaços (ascii 32)
    - [ ] verificar se header existe antes de inserir
    - [ ] lidar com restos no buffer depois de S::Done ou S::Error
    - [x] processar chunked body
    - [x] ignorar trailing headers
    - [ ] overload do feed() com chars
    - [ ] clear() - o que fazer com restos do próximo request no buffer?
- [ ] Implementar testes tabela

# HTTP - Enviar Pedido
# CGI
# Gestão Eventos
# Gestão Clientes

# Logger
- [x] Print básico
- [ ] Colocar traces e debugs

# Compilação
- [ ] Fazer Makefile

# Outros
- [ ] Diagrama
