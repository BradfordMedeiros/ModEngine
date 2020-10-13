(define allowed-messages '(
    "diffuse_on" 
    "diffuse_off"
    "paint_on"
    "paint_off"
    "bloom_on"
    "bloom_off"
  )
)
(define (onMessage message)
  (if (member message allowed-messages)
    (set-state message)
  )
)