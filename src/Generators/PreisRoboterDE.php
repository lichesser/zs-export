<?php
namespace ElasticExport\Generators;

use ElasticExport\Helper\ElasticExportHelper;
use Plenty\Modules\DataExchange\Contracts\CSVGenerator;
use Plenty\Modules\DataExchange\Models\FormatSetting;
use Plenty\Modules\Helper\Services\ArrayHelper;
use Plenty\Modules\Item\DataLayer\Models\Record;
use Plenty\Modules\Item\DataLayer\Models\RecordList;

/**
 * Class PreisRoboterDE
 * @package ElasticExport\Generators
 */
class PreisRoboterDE extends CSVGenerator
{
    /*
     * @var
     */
    private $elasticExportHelper;

    /*
     * @var ArrayHelper
     */
    private $arrayHelper;

    /**
     * IdealoGenerator constructor.
     * @param ElasticExportHelper $elasticExportHelper
     */
    public function __construct(ElasticExportHelper $elasticExportHelper, ArrayHelper $arrayHelper)
    {
        $this->elasticExportHelper = $elasticExportHelper;
        $this->arrayHelper = $arrayHelper;

        //set CSV delimiter
        $this->setDelimiter("|");
    }

    /**
     * @param RecordList $resultData
     * @param array $formatSettings
     */
    protected function generateContent($resultData, array $formatSettings = [])
    {
        if($resultData instanceof RecordList)
		{
            //convert settings to array
			$settings = $this->arrayHelper->buildMapFromObjectList($formatSettings, 'key', 'value');

            //add header to csv
			$this->addCSVContent([
					'art_number',
                    'art_name',
                    'art_price',
                    'art_url',
                    'art_img_url',
                    'art_description',
                    'art_versandkosten',
                    'art_lieferzeit',
                    'art_ean_code',
                    'art_pzn',
                    'art_producer',
                    'art_producer_number',
                    'art_baseprice',
			]);

			foreach($resultData as $item)
			{
			    $deliveryCost = $this->elasticExportHelper->getShippingCost($item, $settings);

			    if(!is_null($deliveryCost))
                {
                    $deliveryCost = number_format((float)$deliveryCost, 2, ',', '');
                }
                else
                {
                    $deliveryCost = '';
                }

				$row = [
					'art_number' 		    => $item->itemBase->id,
					'art_name' 		        => $this->elasticExportHelper->getName($item, $settings, 256),
					'art_price' 	        => number_format((float)$this->elasticExportHelper->getPrice($item), 2, '.', ''),
					'art_url' 		        => $this->elasticExportHelper->getUrl($item, $settings, true, false),
					'art_img_url' 	        => $this->elasticExportHelper->getMainImage($item, $settings),
					'art_description' 	    => $this->elasticExportHelper->getDescription($item, $settings, 256),
					'art_versandkosten'     => $deliveryCost,
	                'art_lieferzeit' 	    => $this->elasticExportHelper->getAvailability($item, $settings),
					'art_ean_code'		    => $item->variationBarcode->code,
					'art_pzn' 	            => '',
					'art_producer' 	        => $this->elasticExportHelper->getExternalManufacturerName((int)$item->itemBase->producerId),
	                'art_producer_number'   => $item->variationBase->model,
	                'art_baseprice' 		=> $this->elasticExportHelper->getBasePrice($item, $settings),
				];

				$this->addCSVContent(array_values($row));
			}
        }
    }
}
