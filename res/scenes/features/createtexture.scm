;
(define (create-obj)
	(format #t "create obj placeholder\n")
   	(mk-obj-attr "someobj"     
  		(list
  			(list "position" (list 1 1 0))
  			(list "mesh" "./res/models/box/spriteplane.dae")
  		)
	)
  (gameobj-setattr! (lsobj-name "someobj/Plane") 
  	(list
  		(list "texture" "graphs-testplot")
  	)
  )
)



(define xRangeSecs 50)
(define yRangeFPS 500)
(define (convertX value) (- (* 2 (/ value xRangeSecs)) 1))
(define (convertY value) (min 1 (max -1 (- (* 2 (/ value yRangeFPS )) 1))))


(define pointX #f)
(define pointY #f)
(define (plotPoint x y)
	(if (and pointX pointY)
		(begin
   		(draw-line (list pointX pointY 0) (list x y 0) #f textureId)
   		(draw-line (list pointX 0.9 0) (list x 0.9 0) #f textureId)

   		(format #t "x = ~a, y = ~a\n" x y)
		)
	)
	(set! pointX x)
	(set! pointY y)
)

(define (plotX)
	(draw-line (list -1 -1 0) (list 1 1 0) #f textureId)
	(draw-line (list -1 1 0) (list 1 -1 0) #f textureId)
)

(define minDrawingTime 0.1)
(define lastTime (time-seconds))
(define (onFrame)
	(define now (time-seconds))
	(define diff (- now lastTime))
	(if (> diff minDrawingTime)
		(begin
			(set! lastTime now)
      (let* ((currSec (time-seconds)) (fps (runstat "fps")) (coordX (convertX currSec)) (coordY (convertY fps)))
				;(format #t "(time = ~a[~a] , fps = ~a[~a])" currSec  coordX fps coordY)
				(plotPoint coordX coordY)
			)
		)
	)
	(draw-text "ModAttributes" 10 10 4 )
	(plotX)
)

(define textureId #f)
(define texturename "graphs-testplot")
(set! textureId (create-texture texturename 1000 1000))

(define (onKeyChar codepoint)
	(if (equal? codepoint 46) (free-texture texturename))           ; . 
	(if (equal? codepoint 47) (create-obj))                         ; / 
)

