# Elementos, GUI e DX no mobile

Revisão em 2026-11-15 na branch `staging`. Os lotes usam a árvore de trabalho
completa, que já continha alterações locais de DX, SVG e motor.

## Progresso das funções Lua

**Geral: 651 de 1.581 funções com implementação no catálogo — 41,2%.**
Elementos: **54/81 — 66,7%**. GUI/DX: **175/199 — 87,9%**.

Este lote corrigiu **5 de 5 problemas da revisão** em funções já registradas.
A cobertura de funções permanece igual. A classificação conta handlers com
corpo não constante; não comprova paridade completa nem validação no aparelho.

## Implementado neste lote

| Área | Alteração | Limite atual |
|---|---|---|
| Elementos | `isElementAttached`, `getElementAttachedTo`, `getAttachedElements` e `getElementAttachedOffsets` | Consultam os dados de `ENTITY_ADD`; não criam anexação física. |
| Referências | Resolvem elementos declarados depois no mesmo pacote e limpam vínculos na remoção | Atualizações posteriores por RPC ainda não são aplicadas. |
| GUI edit | Inserção e backspace na posição do cursor, contagem por caracteres Unicode, respeito a somente leitura | Seleção de texto e posicionamento pelo toque ainda precisam de implementação. |
| GUI memo | Foco, digitação Unicode, backspace, Enter e desenho por linha; newline final e cursor seguem CEGUI | Faltam seleção, quebra automática de linha e rolagem automática do cursor. |
| Limite de texto | `guiSetText` e `guiEditSetMaxLength` preservam caracteres completos; limite zero aceita texto vazio | Digitação mantém o limite adicional de 4096 bytes do mobile. |
| JNI | Conversão UTF-16/UTF-8 para entrada do teclado, medição e envio do frame DX | Outras interfaces JNI não foram revisadas neste lote. |
| DX 2D | `postGUI` nas oito funções de desenho 2D, com blend capturado e comandos de fonte junto do texto | O texto Canvas usa alinhamento, clipping, quebra, escalas por eixo e rotação; métricas ainda diferem do desktop. |
| Retornos DX | `dxDrawRectangle` e `dxDrawText` agora retornam `true`/`false` e recusam números não finitos | Registradas antes, essas funções retornavam zero valores Lua. |
| Texturas | `dxDrawImage`/`dxDrawImageSection` aceitam uma textura existente sem exigir dono de recurso | Caminhos de arquivo ainda precisam de recurso para resolução. |
| Composição da GUI | A GUI usa blend normal, independente do último modo DX do script | Desenhos depois da GUI restauram o modo capturado na chamada. |
| Render targets | Desenhos nesses destinos mantêm a sequência original, inclusive quando `postGUI=true` | Não representa equivalência completa com Direct3D. |
| Primitivas | Linhas interpolam as cores dos vértices; entradas não finitas e círculos que transbordam são recusados | Interpolação visual e composição ainda precisam de teste no aparelho. |

Os retornos dos quatro getters seguem `CLuaElementDefs.cpp:1138-1232,1281-1311`.
Os offsets de rotação são expostos em graus. Para um elemento válido sem
anexação, `isElementAttached` retorna `nil`, `getElementAttachedTo` retorna
`false` e `getElementAttachedOffsets` retorna seis valores, como no desktop.
Remover o elemento de destino não reconecta seus filhos se o ID for reutilizado.

O cursor inicial de um edit continua em zero, conforme CEGUI. Alterar seu índice
agora afeta a digitação e o backspace. O máximo padrão corresponde à capacidade
CEGUI exposta como `unsigned int`; um valor Lua negativo segue essa conversão.

## Validação

- `neon_resource_tests`: **257 verificações, zero falhas**. Inclui referências
  futuras, ordem dos filhos, remoção/reuso de ID, offsets em graus, Unicode,
  digitação/backspace de emoji, texto somente leitura, limite zero, camadas DX,
  blend capturado, fonte personalizada, render targets, memo/Enter/CRLF,
  retornos booleanos, geometria inválida e opções de layout de texto.
- `neon_dx_text_tests`: **24 verificações Java, zero falhas** — texto e altura
  total de linha personalizada.
- `neon_dx_state_tests`: **20 verificações Java, zero falhas** — ordem da fila,
  destruição de targets, orçamento e cache de fontes.
- `neon_dx_view_tests`: **12 verificações Java, zero falhas** — executam o
  renderer real com doubles Android para os cinco problemas da revisão.
  Esses testes não verificam glifos ou GPU reais.
- Biblioteca Android ARM64: compilou e linkou com o NDK.
- Classes Java do jogo: compilaram, incluindo o gradiente de linhas.
- `git diff --check`: sem erros de whitespace.

Não houve teste visual no aparelho nem geração de APK novo neste lote.
A compilação nativa usa as alterações anteriores do motor presentes na árvore
de trabalho; a limitação do checkout isolado está em [REVIEW_LUA_MOBILE.md](REVIEW_LUA_MOBILE.md).

## Correções da revisão — 5/5 concluídas

| Problema | Estado | Evidência |
|---|---|---|
| Target destruído era recriado por quadro adiado | Corrigido | Remoção é executada depois dos desenhos anteriores e sobrevive a quadros substituídos. |
| Blend era perdido ao adiar render targets | Corrigido | Cada trecho preserva blend/fonte; a tela seguinte restaura o estado inicial. |
| `add` fazia soma saturada de cores | Corrigido | Composição source-over para a fonte premultiplicada; vermelho opaco sobre azul permanece vermelho. |
| Recriar fonte mantinha arquivo/medidas antigos | Corrigido | Identidade por criação em ambos os caches, com limite LRU no Java. |
| `lineHeight` não contava a última linha | Corrigido | Altura do bloco e avanço explícito de linhas seguem o desktop. |

## Próximas pendências

1. **Elementos:** portar `attachElements`, `detachElements` e
   `setElementAttachedOffsets` com chamadas físicas e RPCs, antes de considerar
   anexação compatível de ponta a ponta. Os dois primeiros ainda são stubs e
   agora retornam `false` porque não executam a operação física.
   Faltam também consultas espaciais, bounding box, raio e integração completa
   das transformações com o corpo nativo.
2. **GUI:** seleção, posicionamento do cursor pelo toque, propriedades CEGUI,
   fonte customizada, quebra/rolagem automática de memo e ordenação efetiva
   de gridlist. Funções registradas podem ter
   comportamento parcial; o catálogo não comprova equivalência visual.
3. **DX:** comparar métricas e quebras de texto no aparelho, ampliar fontes
   nativas e portar desenho 3D com câmera/profundidade, shaders e efeitos de modelo 2DFX.
4. **Aparelho:** verificar cursor no meio do texto, acentos/emoji, composição
   com transparência, gradientes, GUI antes/depois do DX e desenho em texturas.

## Catálogo atualizado

A [revisão dos lotes](REVIEW_ELEMENTS_GUI_DX.md) registra problemas
corrigidos, evidências e limites de validação. Os lotes seguintes completam
comportamentos de funções já registradas; a contagem de bindings permanece igual.

O layout DX usa o comando `X`, separado do comando `T` da GUI. Seu texto é
escapado antes de atravessar a ponte JNI; seleção de fonte e reset seguem na
mesma fila. O renderer Android usa `StaticLayout`, com largura máxima de 32768
pixels. Escalas não positivas não desenham. Os testes Java verificam a geometria
e o payload; não verificam os glifos ou a composição no aparelho.

| Categoria | Implementadas/total no catálogo | Cobertura | Stubs | Ausentes |
|---|---:|---:|---:|---:|
| Veículos | 9/165 | **5,5%** | 19 | 137 |
| Peds e jogadores | 27/206 | **13,1%** | 77 | 102 |
| Mundo e objetos | 124/227 | **54,6%** | 11 | 92 |
| Elementos | 54/81 | **66,7%** | 8 | 19 |
| GUI e DX | 175/199 | **87,9%** | 0 | 24 |
| Motor e modelos | 1/125 | **0,8%** | 15 | 109 |
| Áudio e voz | 21/57 | **36,8%** | 1 | 35 |
| Câmera | 47/47 | **100,0%** | 0 | 0 |
| Armas e projéteis | 20/31 | **64,5%** | 0 | 11 |
| Sistema e utilitários | 169/195 | **86,7%** | 2 | 24 |
| Markers, blips e colshapes | 0/145 | **0,0%** | 3 | 142 |
| Browser e compatibilidade | 4/103 | **3,9%** | 0 | 99 |
| **TOTAL** | **651/1581** | **41,2%** | **136** | **794** |

Os números incluem as alterações locais anteriores. “Implemented” no catálogo
significa apenas que há uma função C com corpo não constante; não significa
paridade completa. As restrições deste lote também foram registradas nas
[consultas de elementos](lua-functions/ELEMENTS.md) e na
[categoria GUI/DX](lua-functions/GUI_AND_DX.md).
