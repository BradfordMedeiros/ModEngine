
(define players '())

;(define (create-player name)
;  (set! players (cons name "placeholder-id"))
;)
;(define (delete-player name)
;  (set! players (assoc-remove! players name))
;)

;(define (on-player-join connection-hash)
;  (create-player connection-hash)
;)
;(define (on-player-leave connection-hash)
;  (delete-player connection-hash)
;)

;(define (move-player player keyVec2 mouseVec2)
;  (assoc-ref players player)
;  (display "move placeholder")
;)


;(define (onKey key scancode action mods)
;  (display "on key placeholder")
  ; get the direction here
;)
;(define (onMouseMove)
;  (display "\n")
;)
;(define (get-key-vec2)
;  '(0 0)
;)
;(define (get-key-vec3)
;  '(0 0 0)
;)

;(define (get-player1)
;  (list (list 0 0) (list 0 0 0))
;)

(define (onFrame)
  ;(let ((playerinput (get-player1)))
  ;  (move-player (car playerinput) (cadr playerinput))
  ;)
  (display "hello world\n")
)

