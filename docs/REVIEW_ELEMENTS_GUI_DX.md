# Revisão de elementos, GUI e DX

Revisão em 2026-11-15, branch `staging`. Escopo: os dois lotes de elementos,
GUI e DX deste chat. O commit inclui a base de DX/SVG necessária para essas
funções. As alterações anteriores de motor, rede e launcher permanecem locais.

## Problemas encontrados e corrigidos

| Prioridade | Problema | Correção e evidência |
|---|---|---|
| P2 | `dxDrawRectangle` e `dxDrawText` retornavam zero valores, quebrando scripts que verificam sucesso | Retornam booleano; entradas inválidas retornam `false`. Asserções Lua cobrem sucesso, tipo inválido, infinito e flag inválida. |
| P2 | Texto, retângulos e imagens ignoravam `postGUI`; a fonte de um texto adiado podia ficar fora da sua camada | As oito funções 2D compartilham o enfileiramento. Blend é capturado na chamada, e seleção/reset de fonte seguem o texto. Testes verificam as duas camadas, fonte personalizada e desenhos em render targets. |
| P2 | Uma textura existente era recusada por `dxDrawImage` se o chamador não tivesse dono de recurso | Elementos de textura são resolvidos independentemente do dono; caminhos continuam vinculados a um recurso. O teste desenha um render target fora de um recurso. |
| P2 | As consultas de elementos novas recusavam texturas, fontes e SVG válidos | `KnownElement` reconhece `DxElementType`, que também cobre SVG. Asserções verificam consultas de anexação em um render target vivo. |
| P2 | `attachElements` e `detachElements` informavam sucesso sem alterar corpo ou metadados | Os stubs retornam `false`. Asserções comprovam que não prometem uma mutação que o mobile ainda não executa. A anexação física continua pendente. |
| P2 | Erros de argumento das primitivas chamavam `luaL_error` com buffers C++ vivos; o `longjmp` podia pular seus destruidores | A construção dos comandos retorna a um handler sem `vector`/`string` vivos antes de levantar o erro. Leitura de tabelas ignora strings, como `ReadNumberTable` desktop. Asserções cobrem erro depois de um vértice válido e retomada pelo Lua; a liberação foi conferida por inspeção do fluxo, sem medição de heap em aparelho. |
| P2 | Uma exceção de Canvas ao desenhar primitivas com material deixava o bitmap shader no Paint compartilhado | `try/finally` limpa o shader. O Java compila; injeção dessa falha e validação visual ainda dependem do Android. |
| P2 | Memo não participava de foco/teclado; Enter e cursor não seguiam o controle multilinha | Memo aceita Unicode, Enter e backspace, preserva o newline final de CEGUI e emite Changed sem Accepted para Enter. Índices negativos seguem conversão unsigned e clamp. Testes cobrem somente leitura, cursor, CRLF, linhas e ocultação. |
| P2 | Normalizar CRLF apenas no código nativo deixava o buffer do teclado com contagem diferente; Enter enviado diretamente também não atualizava esse buffer | O Android normaliza o texto antes de calcular diferenças e inclui Enter no buffer observado. Esse caminho foi revisado e compilado; interação real com IMEs ainda precisa de aparelho. |

Os pontos da primeira etapa também foram conferidos: referências futuras de
anexação, remoção/reuso de ID, rotação em graus, conversão UTF-8/UTF-16 e
isolamento do blend da GUI. Não foram adicionadas escritas físicas no motor.

Na preparação para publicação, a validação SVG passou a descartar os nós XML
temporários e recusar dimensões não finitas ou que virariam zero. O teste verifica
que esses nós não permanecem acessíveis pelos handles XML.

## Validação realizada

- `neon_resource_tests`: **251 verificações, zero falhas**.
- `WorldRPGNeon` ARM64: compilou e linkou com o NDK.
- Classes Java de jogo: compilaram; somente avisos de APIs obsoletas e
  bootstrap class path para Java 8.
- Catálogo de Lua regenerado: 651 handlers não constantes, 136 stubs e
  794 ausentes. O segundo lote corrige comportamento de bindings existentes.
- `git diff --check`: sem erros.

Não houve teste no aparelho nem geração de APK novo. A compilação nativa usa
a árvore completa com alterações anteriores do motor. A limitação do checkout
isolado está em [REVIEW_LUA_MOBILE.md](REVIEW_LUA_MOBILE.md).

## Pendências preservadas

- Anexação física e RPCs de anexação; consultas espaciais, bounding box/raio
  e transformações completas de elementos.
- Seleção de texto, posicionamento do cursor pelo toque, propriedades/fonte
  CEGUI, ordenação real de gridlist e quebra/rolagem automática de memo.
- Layout completo de `dxDrawText`: alinhamento, clipping, quebras, escalas por
  eixo e rotação. A renderização atual usa o formato Canvas de escala única.
- DX 3D com profundidade, shaders e efeitos de modelo 2DFX.
- Validação visual de transparência, gradientes, render targets, teclado e
  eventos de foco nos dispositivos Android de destino.

Detalhes funcionais: [roadmap de elementos, GUI e DX](ROADMAP_ELEMENTS_GUI_DX_MOBILE.md).
