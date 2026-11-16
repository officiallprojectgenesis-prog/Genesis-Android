# Próximas conversões Lua sugeridas

Sugestão após a revisão GUI/DX de 2026-11-15. Este documento propõe trabalho;
as funções abaixo continuam ausentes ou parciais no catálogo. A prioridade
considera as dependências vistas neste checkout, sem um levantamento de todos
os scripts usados pelo servidor.

| Ordem | Funções | Benefício | Dependências e critério de conclusão |
|---|---|---|---|
| 1 | `getElementsWithinRange` | Busca de jogadores, veículos e objetos próximos para interações | Portar a consulta espacial e seus filtros de tipo, interior e dimensão. Usar posições atuais, remoção/reuso de IDs e elementos locais/recebidos do servidor. Referência: `CLuaElementDefs.cpp:972-1015`; a busca desktop não equivale em todos os casos a testar só a distância do centro. |
| 2 | `createColSphere`, `createColCuboid`, `isElementWithinColShape`, `getElementsWithinColShape` | Zonas de lojas, portas, garagens e entradas de painéis | Criar armazenamento real de formas e manter a lista de colisões, com dimensões/interiores, movimento, remoção e limpeza por recurso. Ligar `onClientColShapeHit`/`onClientColShapeLeave`. As consultas desktop usam essa lista; não basta fabricar um elemento. Referências: `CLuaColShapeDefs.cpp:152-239`, `CLuaElementDefs.cpp:875-970` e `CClientColShape*`. |
| 3 | `createColCircle`, `createColRectangle`, `createColTube` | Completar as formas simples usando o mesmo subsistema do lote anterior | Conferir limites 2D/3D e altura, bordas e retorno dos eventos. A geometria pode ter testes determinísticos; posições/eventos ainda exigem integração. Referência: `CLuaColShapeDefs.cpp:109-150,241-286,339-386`. |
| 4 | `call`, `downloadFile` | Comunicação entre recursos e carregamento de arquivos sob demanda | `call`: exports, ambiente, recurso parado e permissões. `downloadFile`: caminho permitido, geração do recurso, cancelamento/stop, resultado real do download e evento de conclusão. Reaproveitar os sistemas de recursos/HTTP existentes. Não há necessidade de novos offsets GTA para a parte principal desse lote. |
| 5 | `engineLoadTXD`, `engineImportTXD`, `engineLoadDFF`, `engineReplaceModel` | Mapas, veículos, objetos e skins personalizados | Impacto alto, mas requer ciclo de vida de texturas/modelos, streaming, colisão e restauração no stop. Resolver símbolos mobile e testar carregamento, troca e descarregamento no aparelho antes de declarar compatibilidade. |

Minha escolha para o próximo lote é **1 + 2**: cinco funções, com uma base
coerente para proximidade e zonas de interação. O parser atual de `ENTITY_ADD`
já reconhece as formas, mas ainda descarta parte das dimensões; é necessário
preservar esses dados e portar o ciclo de colisão. Geometria e filtragem podem
ser verificadas no host antes do teste de movimento/eventos no Android.

Em paralelo às novas funções, ainda há trabalho nas já registradas: seleção e
rolagem da GUI, máscara para `overwrite` em shaders, texturas de shader atualizadas
ao vivo, includes/estados FX, estágios 3D e efeitos 2DFX persistentes. A cobertura
199/199 de GUI/DX mede handlers, não encerramento dessas pendências.
